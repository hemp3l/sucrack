/*
 * Copyright (c) 2006, Nico Leidecker
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organization nor the names of its contributors 
 *       may be used to endorse or promote products derived from this software 
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include "stat.h"
#include "dictionary.h"
#include "util.h"
#include "worker.h"
#include "rewriter.h"

#ifndef STATIC_BUFFER
    #ifndef DYNAMIC_LIST
	#define STATIC_BUFFER
    #endif
#endif

/* word list buffer */
#ifdef STATIC_BUFFER
static char **dict_buffer;
#else
static dict_word_list *dict_buffer;

/* pointer to entry of last extracted word */
static dict_word_list *out_next;

/* pointer to entry of last inserted word */
static dict_word_list *in_next;

/* mutex for word list access */
static pthread_mutex_t	dmutx = PTHREAD_MUTEX_INITIALIZER;;
#endif

/* max size of word list buffer */
static int dict_bufsz_max;

/* number of words in buffer */
static int dict_bufsz;

/* dictionary file handle */
static FILE *dictfd;

/* eof flag */
static int dict_eof;

/* rewriter status (1 = enabled, 0 = disabled) */
static int rewr;

/* dictionary thread handle */
static pthread_t dict_thread;

static stat_dict_t dstat;

static FILE *
dict_open(char *file)
{
    struct stat st;
    FILE *fd;
    
    fd = fopen(file, "r");
    if (!fd) {
	sys_err_printf("dict_open: fopen");
	return 0;
    }
    
    if (fstat(fileno(fd), &st) == -1) {
	sys_err_printf("dict_open: fstat");
	return 0;
    }
    
    dstat.fsize = st.st_size;
    
    return fd;
}

/* in order to minimize the overhead, we generate the dicionary word list structure here for 
 *  once and just fill it with values on time.  The structure is actually a ring as the last element 
 *  points to the first
 */
int
dict_init(char *file, unsigned int size, int r)
{

#ifndef STATIC_BUFFER 
    dict_word_list *tmp;
#endif

    dict_eof = 0;
    rewr = r;
    
#ifdef STATISTICS
    memset(&dstat, 0x00, sizeof(stat_dict_t));    
    dstat.rewr = r;
#endif

    if (strcmp(file, "-") == 0) {
	debug_printf("dictionary reading from stdin\n", file);
	dictfd = stdin;
    } else {
	debug_printf("open dictionary file '%s' for reading\n", file);
	dictfd = dict_open(file);
	if (!dictfd) {
	    fatal_printf("dictionary file could not be opened\n");
	    return -1;
	}
    }
    
    debug_printf("generating word list with %i entries\n", size);
    
    dict_bufsz_max = size;
    
#ifdef STATISTICS
    dstat.bsize = size;
#endif

#ifdef STATIC_BUFFER 
    dict_buffer = (char **) malloc(sizeof(char *) * size);
    
    while(--size) {
	dict_buffer[size] = 0;
    }
    
#else
    dict_buffer = (dict_word_list *) malloc(sizeof(dict_word_list));
    tmp = dict_buffer;
    
    while (--size) {
	tmp->word = 0;
	tmp->next = (dict_word_list *) malloc(sizeof(dict_word_list));
	if (!dict_buffer) {
	    sys_err_printf("dict_init: malloc");
	    return -1;
	}
	tmp = tmp->next;
	tmp->next = 0;
    }
    tmp->next = dict_buffer;
    
    out_next = dict_buffer;
    in_next = dict_buffer;
    
#endif
    dict_bufsz = 0;

    if (rewr) {
	rewr_init();
    }
    
    if (pthread_create(&dict_thread, 0, dict_run, 0) != 0) {
	sys_err_printf("dict_init: pthread_create");
	return -1;
    }

    return 0;
}

#ifndef STATIC_BUFFER
static int 
dict_update_buffer(int op, char *word)
{
    int ret;

    debug_printf("dict: acquiring update mutex\n");
    if (pthread_mutex_lock(&dmutx) != 0) {
	sys_err_printf("dict_update_buffer: pthread_mutex_lock");
	return -1;
    }
    
    debug_printf("operation is %s\n", op == DICT_CRITICAL_OP_READ ? "read" : "write");
    
    if (op == DICT_CRITICAL_OP_READ) {
	if (!word) {
	    err_printf("attempt to get a string into non allocated memory\n");
	    ret = -1;
	} else {
	    if (!dict_bufsz) {
		ret = 0;
	    } else {
		strncpy(word, out_next->word, DICT_MAX_WORD_SIZE);
		free(out_next->word);
	    	out_next->word = 0;
	        out_next = out_next->next;
		dict_bufsz--;
	    }
	}
    } else {
	if (dict_bufsz == dict_bufsz_max) {
	    debug_printf("no space left in word list: current=%i, max=%i\n", dict_bufsz, dict_bufsz_max);
	    ret = 0;
	} else {
	    in_next->word = strdup(word);
	    if (!in_next->word) {
		sys_err_printf("dict_add_word: strdup");
		ret = -1;
	    } else {
		debug_printf("word added to list: %s\n", in_next->word);
		in_next = in_next->next;    
		dict_bufsz++;
	    }
	}
    }
    
    debug_printf("dict: releasing update mutex\n");
    if (pthread_mutex_unlock(&dmutx) != 0) {
	sys_err_printf("dict_update_buffer: pthread_mutex_unlock");
	return -1;
    }

    return ret;
}
#endif

/*
 * Add a word to the dictionary word list, if there is space left.
 * Then, increase the number of words in list and update the pointer
 * to the last inserted word.
 */ 
int
dict_add_word(char *word)
{
#ifdef STATIC_BUFFER
    static int s = 0;
    char *w;
    int l, i;
#endif 

    if (!dict_buffer) {
	fatal_printf("dictionary buffer was not initialized\n");
	return -1;
    }

#ifdef STATIC_BUFFER
    w = strdup(word);
    l = strlen(w);
    if (l > 0) {
	if (w[l - 1] == '\n') {
	    w[l - 1] = '\0';
	}
    }

    i = dict_bufsz_max;
    while(i) {
	if (!dict_buffer[s]) {
	    dict_buffer[s] = w;
	    debug_printf("add '%s' to dictionary buffer space %i; %i elements in buffer\n", word, s, ++dict_bufsz);
	    return 1;
	}
	s = (s + 1) % dict_bufsz_max;
	i--;
    }
    
    return 0;
#else
    return dict_update_buffer(DICT_CRITICAL_OP_WRITE, word);
#endif

}

/*
 * Get a word from the dictionary word list, if there is any.
 * Then, update the pointer to the word to be removed and decrease
 * the number of words remaining in the list.
 */ 
char *
dict_get_word(unsigned int id)
{
#ifndef STATIC_BUFFER
    char word[DICT_MAX_WORD_SIZE + 1];
#endif
    char *ret;
    
    debug_printf("dict: going to get word from dictionary buffer\n");
    
#ifdef STATIC_BUFFER

    debug_printf("dict: buffer list place id is %i/%i\n", id, dict_bufsz_max);
    if (id > dict_bufsz_max || !dict_buffer[id]) {
	return 0;
    }
   
    ret = strdup(dict_buffer[id]);
    dict_buffer[id] = 0;
    dict_bufsz--;
#else

    if (!dict_bufsz) {
	return 0;
    }
    
    dict_update_buffer(DICT_CRITICAL_OP_READ, word);
    ret = strdup(word);

#endif

    debug_printf("dict: got %s from dictionary buffer; %i remaining\n", ret, dict_bufsz);
    
    return ret;
}

/*
 * Update word list buffer.
 * If list is full, wait some time and try again.
 * But do not wait more than `max_wait * DICT_UPDATE_WAIT_US' microseconds
 * max_wait = -1 is infinite waiting
 */
int
dict_update_buffer_wait(int max_wait)
{
    char word[DICT_MAX_WORD_SIZE + 1];
    char *aw;
    int ret;
    clock_t then;
    rule_set_t *rules;
    
    
    debug_printf("dict: updating word list buffer\n");
        
    if (!(fgets(word, DICT_MAX_WORD_SIZE, dictfd))) {
	return 0;
    }

    if (strlen(word) > 0) {
	if (word[strlen(word) - 1] == '\n') {
	    word[strlen(word) - 1] = '\0';
	}
    }

    then = time(0);	
    while(!(ret = dict_add_word(word)) && (max_wait == -1 || max_wait-- > 0)) {
	usleep(DICT_UPDATE_WAIT_US);
    }
    dstat.bnum += strlen(word);
    
#ifdef STATISTICS
    dstat.twadd += time(0) - then;
    dstat.wnum++;
#endif

    if (!ret) {
	debug_printf("dict: buffer updating operation timed out: skip word\n");
	return ret;
    }

    /*
     * rewrite the words and add it to the word list
     */ 
    

    if (rewr) {
	debug_printf("dict: rewriting %s\n", word);
	rules = rewr_get_rules();
        while((aw = rewr_rewrite(word, rules))) {
	    debug_printf("dict: rewrote %s to %s\n", word, aw);
	    if (strcmp(word, aw) != 0) {
    		then = time(0);
		while(!(ret = dict_add_word(aw)) && (max_wait == -1 || max_wait-- > 0)) {
		    usleep(DICT_UPDATE_WAIT_US);
		}
#ifdef STATISTICS
		dstat.rbnum += strlen(aw);
		dstat.twadd += time(0) - then;
		dstat.wnum++;
#endif
	    }
	    free(aw);
	}
	free(rules);
    }
    
    return ret;
}

void *
dict_run(void *args)
{
    while(dict_update_buffer_wait(-1)) {
	usleep(100);
    }

    dict_eof = 1;
    
    pthread_exit(0);
}

void
dict_kill(int force)
{
    if (force) {
	pthread_cancel(dict_thread);
    }
    pthread_join(dict_thread, 0);
}

int
dict_is_eof()
{
    return dict_eof;
}

void
dict_get_stats(stat_dict_t **stats)
{
    *stats = &dstat;
}
