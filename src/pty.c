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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pty.h"
#include "util.h"

#ifndef LINUX

static const char *cmap[] = {
    "pqrs",
    "0123456789abcdefghijklmnopqrstuv"
};


int
get_pty(int *mfd, int *sfd)
{
    char mdev[DEVICE_NAME_SIZE + 1];
    char sdev[DEVICE_NAME_SIZE + 1];
    char *mp, *sp;
    int dx, dy;
    
    strcpy(mdev, PTY_DEVICE_NAME);
    mp = mdev + strlen(PTY_DEVICE_NAME) - 2;

    strcpy(sdev, TTY_DEVICE_NAME);
    sp = sdev + strlen(TTY_DEVICE_NAME) - 2;

    for (dx = 0; dx < strlen(cmap[0]); dx++) {
	mp[0] = cmap[0][dx];
	for (dy = 0; dy < strlen(cmap[1]); dy++) {
	    mp[1] = cmap[1][dy];
	    *mfd = open(mdev, O_RDWR);
	    if (*mfd != -1) {
		sp[0] = mp[0];
		sp[1] = mp[1];
		*sfd = open(sdev, O_RDWR);
		if (*sfd != -1) {
		    debug_printf("attached %s (%i), %s (%i)\n", mdev, *mfd, sdev, *sfd);
		    return 1;
		} else {
		    sys_err_printf("get_pty: open");
		}
	    } else {
		debug_printf("get_pty: open");
	    }
	} 
    }
    return -1;
}

#else
int
get_pty(int *mfd, int *sfd)
{
    char sdev[64];

    debug_printf("opening master device: %s\n", PT_MASTER_DEVICE_NAME);
    *mfd = open(PT_MASTER_DEVICE_NAME, O_RDWR);
    if(*mfd != -1) {
	grantpt(*mfd);
	unlockpt(*mfd);
	if (ptsname_r(*mfd, sdev, sizeof(sdev)) != 0) {
	    sys_err_printf("get_pty: ptsname");
	    return -1;
	}
	debug_printf("opening slave device: %s\n", sdev);
	*sfd = open(sdev, O_RDWR);
	if (*sfd != -1) {
	    debug_printf("attached %s (%i), %s (%i)\n", PT_MASTER_DEVICE_NAME, *mfd, sdev, *sfd);
	    return 1;
	} else {
	    sys_err_printf("get_pty: open slave");
	}
    } else {
        sys_err_printf("get_pty: open master");
    }
		
    return -1;
}

#endif
