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

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DEBUG_OUT		stdout
#define ERROR_OUT		stderr
#define	SYS_ERROR_OUT		stderr
#define FATAL_OUT		stderr
#define NOTICE_OUT		stdout

#ifdef DEBUG
    #define debug_printf(fmt, args...)	fprintf(DEBUG_OUT, "DEBUG: " fmt, ##args);
#else 
    #define debug_printf(fmt, args...)	
#endif

#define err_printf(fmt, args...)	fprintf(ERROR_OUT, "ERROR: " fmt, ##args);

#define notice_printf(fmt, args...)	fprintf(NOTICE_OUT, "NOTICE: " fmt, ##args);

#define sys_err_printf(fmt, args...)	fprintf(SYS_ERROR_OUT, "SYSTEM: " fmt ": %s\n", ##args, strerror(errno));

#define fatal_printf(fmt, args...)	fprintf(FATAL_OUT, "FATAL: " fmt, ##args);

//void	sys_err_printf(char *fmt, ...);

void	prepare_tty();
void	reset_tty();
int	key_pressed();

#endif

