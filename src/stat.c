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

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "sucrack.h"
#include "stat.h"
#include "worker.h"
#include "dictionary.h"
#include "su.h"
#include "rewriter.h"

#ifdef STATISTICS
static void
pretty_print_time(int sec)
{
    int s, m, h, d;
    
    s = sec % 60;
    sec /= 60;
    m = sec % 60;
    sec /= 60;
    h = sec % 24;
    d = sec;
    
    if (d > 0) {
	printf("%i day%s ", d, d > 1 ? "s": "");
    }
    
    printf("%.2i:%.2i:%.2i\n", h, m, s);
}

void
stat_display(int intv, int ansi)
{
    struct winsize ws;
    static clock_t start = 0;
    static clock_t then = 0;
    clock_t now;
    stat_dict_t *dstat;
    stat_worker_t *wstat;
    stat_su_t *sstat;
    stat_rewr_t *rstat;
    int p, pdone;
    float prgr;
    
    // time...
    if (!start) {
	start = time(0);
    }
    
    now = time(0);
    if (abs(now - then) < intv && then != 0) {
	return;
    }
    then = now;
    
    if (ioctl(0, TIOCGWINSZ, &ws) == -1) {
	ws.ws_row = 25;
	ws.ws_col = 80;
    }
    
    if (ansi) {
	printf("\33[0;0H\33[2J");
        printf("\33[0;%iH\33[1m%s\33[0m\n\n\n", (ws.ws_col / 2) - ((strlen(SUCRACK_TITLE) + 7)/ 2), SUCRACK_TITLE);
    }
    
    
    dict_get_stats(&dstat);
    su_get_stats(&sstat);
    worker_get_stats(&wstat);
    
    printf("     time elapsed:    ");
    pretty_print_time((unsigned int) (time(0) - start));
    
    prgr = (float) dstat->bnum / (float) dstat->fsize;
    printf("   time remaining:    ");
    pretty_print_time(((float) time(0) - (float) start) * (1.0 - prgr) / prgr); 
    
    printf("         progress:    %.2f%% [", prgr * 100.0);
    pdone = (int) ((prgr * (ws.ws_col / 2)) + 1);
    for (p = 0; p < ws.ws_col / 2; p++) {
	printf("%c", p < pdone -1 ? '*' : '.');
    }
    printf("]\n");
    printf("     user account:    %s\n\n", sstat->user);
    
    printf(" __dictionary:_______________________\n");
    printf("        file size:    %i\n", dstat->fsize);
    printf("       bytes read:    %i\n", dstat->bnum);
    printf("       words read:    %i\n", dstat->wnum);
    printf(" word buffer size:    %i\n", dstat->bsize);
    printf("    time/word add:    %.4f\n", (float) dstat->twadd / (float) dstat->wnum);
    if (dstat->rewr) {
	rewr_get_stats(&rstat);
	printf("   rewriter rules:    %i (%.4x)\n", rstat->rnum, rstat->rules);
	printf("  bytes rewritten:    %i\n", dstat->rbnum);
    } else {
	printf("         rewriter:    disabled\n");
    }
    printf("\n");
    
    printf(" __worker:___________________________\n");
    printf("           worker:    %i\n", wstat->wnum);
    printf("         attempts:    %i\n", wstat->anum);
    printf("  attempts/worker:    %i\n", wstat->anum / wstat->wnum);
    printf("  seconds/attempt:    %f\n", (float) wstat->tatt / (float) wstat->anum);
//!TODO: is only theoretical v
    printf("     attempts/sec:    %f\n", (1.0 / ((float) wstat->tatt / (float) wstat->anum) * wstat->wnum));
    
    printf("  overhead/worker:    %f\n", 
		    ((float) wstat->tatt / (float) wstat->anum) 
			    - ((float) sstat->tatt / (float) sstat->anum));
    printf("\n");
}

#else 

void
stat_display()
{
    stat_dict_t *dstat;
    dict_get_stats(&dstat);
    printf("%i/%i\n", dstat->bnum, dstat->fsize);
}

#endif
