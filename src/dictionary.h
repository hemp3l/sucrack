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

#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#define DICT_MAX_WORD_SIZE		32
#define DICT_UPDATE_WAIT_US		100

#define DICT_CRITICAL_OP_READ		0
#define DICT_CRITICAL_OP_WRITE		1

#include "stat.h"

typedef struct _dict_word_list dict_word_list;

struct _dict_word_list {
    char *word;
    dict_word_list *next;
};

/*
 * generate dictionary list structure and rewriter/spawn dictionary thread
 * -1 on error, 0 on success 
 */
int	dict_init(char *file, unsigned int size, int r);

/* cancel dictionary thread */
void	dict_kill(int force);

/* add word to list */
int	dict_add_word(char *word);

/* get word from word list */
char   *dict_get_word(unsigned int id);

/* see dict_add_word for return values */
int	dict_update_buffer_wait(int max_wait);

void	dict_get_stats(stat_dict_t **stats);

/* thread main routine */
void   *dict_run(void *args);

/* returns 1 if end of file is reached */
int 	dict_is_eof();

#endif
