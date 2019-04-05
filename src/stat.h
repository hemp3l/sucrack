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

#ifndef _STAT_H
#define _STAT_H


#define STAT_INTERVALL	2.0

typedef struct _stat_dict_t {
    unsigned int fsize;
    unsigned int bnum;
    unsigned int wnum;		// number of words added to list
    unsigned int rbnum;		// number of bytes from rewriter
    unsigned int twadd;		// sum time / add word to line (sec)
    unsigned int bsize;
    unsigned int rewr;
} stat_dict_t;

#ifdef STATISTICS
typedef struct _stat_worker_t {
    unsigned int wnum;		// number of workers
    unsigned int anum;		// number of attempts done
    unsigned int tatt;		// sum time / attempt
    unsigned int wait;
} stat_worker_t;

typedef struct _stat_su_t {
    unsigned int anum;
    unsigned int tatt;
    unsigned int wait;	
    char *user;	
} stat_su_t;

typedef struct _stat_rewr_t {
    unsigned int rnum;
    unsigned int rules;
} stat_rewr_t;

void	stat_display(int intv, int ansi);
#else
void	stat_display();
#endif

#endif

