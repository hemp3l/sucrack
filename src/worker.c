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

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "worker.h"
#include "dictionary.h"
#include "util.h"
#include "su.h"
#include "pty.h"
#include "stat.h"

static worker **workers;
static unsigned short wnum;
static unsigned short walive;
static unsigned int terminate;
#ifdef STATISTICS
static stat_worker_t wstat;
#endif

int
worker_init(unsigned short num)
{

    terminate = 0;    
    
#ifdef STATISTICS
    memset(&wstat, 0x00, sizeof(stat_worker_t));
    wstat.wnum = num;
#endif    

    if (num < 1) {
	fatal_printf("at least one worker has to be initialized\n");
	return -1;
    }
    
    wnum = num;
    
    workers = (worker **) malloc(sizeof(worker *) * num);
    if (!workers) {
	sys_err_printf("[%i] spawn_workers: malloc", num);
	return -1;
    }
    
    walive = 0;
    while(num--) {
	workers[num] = (worker *) malloc(sizeof(worker));
	if (!workers[num]) {
	    sys_err_printf("[%i] spawn_workers: malloc", num);
	    err_printf("[%i] not enough memory to initialize: skipping worker\n", num);
	    continue;
	}
	
	memset(workers[num], 0x00, sizeof(worker));
	
	if (get_pty(&(workers[num]->mfd), &(workers[num]->sfd)) == -1) {
	    err_printf("[%i] could not attach any pseudo terminal: skipping worker\n", num);
	    free(workers[num]);
	    workers[num] = 0;
	    continue;
	}
	walive++;
	
	workers[num]->id = num;
	debug_printf("[%i] initialized\n", num);
    }
        
    debug_printf("worker environment initialized (%i of %i workers initialized)\n", walive, wnum);
    
    return 0;
}

int
worker_spawn()
{
    unsigned short num;
    
    if (!workers) {
	fatal_printf("workers were not initialized\n");
	return -1;
    }
    
    num = wnum;
    while(num--) {
	if (workers[num]) {
	    if (pthread_create(&(workers[num]->pt), 0, worker_run, (void *) workers[num]) != 0) {
		sys_err_printf("[%i] spawn_workers: pthread_create", num);
		return -1;
	    }
	    debug_printf("[%i] spawned\n", num);
	} else {
	    debug_printf("[%i] uninitialized: not spawned\n", num);
	}
    }
    
    return 0;
}

void
worker_kill(int force)
{
    unsigned short num;

    terminate = 1;
    
    num = wnum;
    while(num--) {
	if (workers[num]) {
	    if (force) {
		pthread_cancel(workers[num]->pt);
	    }
	    pthread_join(workers[num]->pt, 0);
	    debug_printf("worker %i killed\n", num);
	} else {
	    debug_printf("worker %i not alive\n", num);
	}
    }
}

void *
worker_run(void *args)
{    
    worker *me;
    clock_t then;
    int rs;
    
    me = (worker *) args;
    
    debug_printf("[%i] running\n", me->id);

    while(1) {
	then = time(0);

	if (me->word) {
	    free(me->word);
	}

	while (!(me->word = dict_get_word(me->id))) {
	    if (terminate) {
		pthread_exit(0);
	    }
	    usleep(100);
	}

	debug_printf("[%i] working on: %s\n", me->id, me->word);
	
	rs = su_run(me->mfd, me->sfd, me->word);
	
	debug_printf("[%i] returned from su with %i\n", me->id, rs);	
#ifdef STATISTICS	
	wstat.tatt += time(0) - then;
	wstat.anum++;
#endif
    }
    
    pthread_exit(0);
}

#ifdef STATISTICS
void
worker_get_stats(stat_worker_t **stats)
{
    *stats = &wstat;
}
#endif

