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

#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#include "su.h"
#include "util.h"
#include "stat.h"

static char *auth_failure, *auth_success;

static char **su;

#ifdef STATISTICS
static stat_su_t sstat = { 0 };
#endif

/*
 * read from file and expect a string / with timeout
 */
static char *
su_busy_read(int fd, char **break_on, int blen, int tm)
{
    char *tbuf;
    char buf[BUSY_READ_BUFSZ + 1];
    struct pollfd fds[1];
    clock_t then;
    int i, tlen, clen, rs;
    
    memset(fds, 0x00, sizeof(struct pollfd));
    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLPRI;
    then = time(0);
    
    clen = 0;
    tlen = 1 + BUSY_READ_BUFSZ * 2;
    tbuf = (char *) malloc(tlen);
    tbuf[0] = '\0';
    
    while (time(0) - then < tm || tm == -1) {
	rs = poll(fds, 1, tm == -1 ? tm : tm * 1000);
	switch(rs) {
	    case  1:
		clen = read(fd, buf, BUSY_READ_BUFSZ);
		if (clen != -1) {
		    buf[clen] = '\0';
		    for (i = 0; i < blen; i++) {
			if ((strlen(tbuf) + clen) >= tlen) {
			    tlen += clen;
			    tbuf = (char *) realloc(tbuf, tlen);
			}
			strncat(tbuf, buf, clen);
			debug_printf("trying '%s' for '%s'\n", tbuf, break_on[i]);
			if (strstr(tbuf, break_on[i])) {
			    free(tbuf);
			    return break_on[i];
		        }
		    }	    
		    break;
		} 
		/* no break: on error go to next case */
	    case -1:
		sys_err_printf("su: su_busy_read (%i)", fd);
	}
    }
    free(tbuf);
    return 0;
}

/*
 * do a su attempt
 */
int
su_run(int mfd, int sfd, const char *pass)
{
    int pid, rs, f;
    char send_buf[SU_MAX_USERNAME_LEN + 2];
    char *resp;
    clock_t then;
        
    debug_printf("su: master=%i slave=%i\n", mfd, sfd);

    then = time(0);

    pid = vfork();
    switch(pid) {
	case 0:
	    close(mfd);
	    dup2(sfd, 0);
	    dup2(sfd, 1);
	    dup2(sfd, 2);
	    execve(*su, su, 0);
	    perror("execve");
	    exit(0);
	case -1:
	    return SU_RETURN_ERROR;
    } 

    debug_printf("su: waiting for prompt\n");

    sprintf(send_buf, "%s\n", pass);
    su_busy_read(mfd, (char *[]) {SU_RESPONSE_PASSWORD}, 1, -1);

    debug_printf("su: sending password: %s\n", send_buf);
    
    f = 0;
    do {
	usleep(10 * f);
	rs = write(mfd, send_buf, strlen(send_buf));
	if (rs == -1) {
	    sys_err_printf("su_run: write");
	    wait(&rs);
	    return SU_RETURN_ERROR;
	}

        resp = su_busy_read(mfd, (char *[]) {auth_failure, auth_success}, 2, 3);
	f *= 10;
    } while(!resp);
    	
    if (strncmp(resp, auth_success, strlen(auth_success)) == 0) {
	printf("password is: %s\n", pass);
	exit(0);
//	return SU_RETURN_SUCCESS;
    } 
    debug_printf("su: waiting for su process to return\n");

    wait(&rs);
#ifdef STATISTICS
    sstat.tatt += time(0) - then;
    sstat.anum++;
#endif

    return SU_RETURN_SORRY;
}

int
su_init(char *user)
{
    int i;
    char *path;

    path = getenv("SUCRACK_SU_PATH");
    if (!path) {
	path = SU_DEFAULT_PATH;
    }
    debug_printf("su: path is %s\n", path);

    auth_failure = getenv("SUCRACK_AUTH_FAILURE");
    if (!auth_failure) {
	auth_failure = SU_RESPONSE_SORRY;
    }
    debug_printf("su: failed authentication response is %s\n", auth_failure);

    auth_success = getenv("SUCRACK_AUTH_SUCCESS");
    if (!auth_success) {
	auth_success = SU_RESPONSE_SUCCESS;
    }
    debug_printf("su: successful authentication response is %s\n", auth_success);

    if (access(path, X_OK) == -1) {
	sys_err_printf("su_init: %s", path);
	return -1;
    }
    
    su = (char **) malloc(5 * sizeof(char *));
    if (!su) {
	sys_err_printf("su_init: malloc");
	return -1;
    }
    for (i = 0; i < 5; i++) {
	su[i] = (char *) malloc(64 * sizeof(char));
	if (!su[i]) {
	    sys_err_printf("su_init: malloc");
	    return -1;
	}
    }
    strncpy(su[0], path, 64);
    strncpy(su[1], user, 64);
    strncpy(su[2], "-c", 64);
    snprintf(su[3], 64, "echo \"%s\"\n", auth_success);
    su[4] = 0;

#ifdef STATISTICS
    sstat.user = su[1];
#endif
    return 0;
}

void
su_user(const char *user)
{
    strncpy(su[1], user, SU_MAX_USERNAME_LEN);
    debug_printf("switch user to %s\n", su[1]);
#ifdef STATISTICS
    sstat.user = su[1];
#endif
}

#ifdef STATISTICS
void
su_get_stats(stat_su_t **stats) {
    *stats = &sstat;
}
#endif
