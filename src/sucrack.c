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
#include <unistd.h>
#include <string.h>
#include <signal.h>

/* local includes */
#include "sucrack.h"
#include "stat.h"
#include "dictionary.h"
#include "worker.h"
#include "su.h"
#include "util.h"
#include "rules.h"
#include "rewriter.h"

/* local termination flag */
static int terminate = 0;

/* capture interrupt signal and set termination flag */
void 
sigint(int i)
{
    terminate = 1;
}

void 
usage(char *path)
{
    printf("%s - the su cracker\n", SUCRACK_TITLE);
    printf("Copyright (C) 2006  Nico Leidecker; nfl@portcullis-security.com\n\n");
    printf(" Usage: %s [-char] [-w num] [-b size] [-s sec] [-u user] [-l rules] wordlist\n\n", path);
    
    printf(" The word list can either be an existing file or stdin. In that case, use '-' instead");
    printf(" of a file name\n\n");
    
    printf(" Options:\n");
    printf("   h       : print this message\n");

#ifdef STATISTICS
    printf("   a       : use ansi escape codes for nice looking statistics\n");
    printf("   s sec   : statistics display interval\n");
    printf("   c       : only print statistics if a key other than `q' is pressed\n");
#else 
    printf("   a       : ansi escape codes not available.\n");
    printf("             Use the --enable-statistics configure flag.\n");
    printf("   s sec   : statistics display interval not available.\n");
    printf("             Use the --enable-statistics configure flag.\n");
    printf("   c       : only print statistics if a key other than `q' is pressed. (default)\n");
#endif
    printf("   r       : enable rewriter\n");
    printf("   w num   : number of worker threads running with\n");
    printf("   b size  : size of word list buffer\n");
    printf("   u user  : user account to su to\n");
    printf("   l rules : specify rewriting rules; rules can be:\n");
    printf("               A = all characters upper case\n");
    printf("               F = first character upper case\n");
    printf("               L = last character upper case\n");
    printf("               a = all characters lower case\n");
    printf("               f = first character lower case\n");
    printf("               l = last character lower case\n");
    printf("               D = prepend digit\n");
    printf("               d = append digit\n");
    printf("               e = 1337 characters\n");
    printf("               x = all rules\n\n");
    printf(" Environment Variables:\n");
    printf("   SUCRACK_SU_PATH      : The path to su (usually /bin/su or /usr/bin/su)\n\n");
    printf("   SUCRACK_AUTH_FAILURE : The message su returns on an authentication\n");
    printf("                          failure (like \"su: Authentication failure\" or \"su: Sorry\")\n");
    printf("   SUCRACK_AUTH_SUCCESS : The message that indicates an authentication\n");
    printf("                          success. This message must not be a password\n");
    printf("                          listed in the wordlist (default is \"SUCRACK_SUCCESS\")\n\n");
    printf(" Example:\n");
    printf("   export SUCRACK_AUTH_SUCCESS=\"sucrack_says_hello\"\n");
    printf("   %s -a -w 20 -s 10 -u root -rl AFLafld dict.txt\n", path);
}

int 
main(int argc, char **argv)
{
    char *dfile, *user;
    int o, ssec, ansi, wnum, bsize, rewr;
    unsigned int rules;
    int read_from_stdin;

    if ((getuid() | getgid()) == 0) {
	printf("yah, verry funny!\n");
	return 0;
    }

    /* initial values */
    dfile = 0;
    user = 0;
    ssec = 0;
    ansi = 0;
    wnum = 1;
    bsize = 2;
    rewr = 0;
    rules = 0x0000;
    read_from_stdin = 0;

    while((o = getopt(argc, argv, "charw:b:s:u:l:")) != -1) {
	switch(o) {
	    case 'c':
		    /*
		     * to set the statistic printing interval to 0 means to not print
		     * statistics unless a key was pressed
                     */
		    ssec = 0;
		    break;
	    case 'h':
		    usage(*argv);
		    return 0;
#ifdef STATISTICS
	    case 'a':
		    ansi = 1;
		    break;
	    case 's':
		    ssec = atoi(optarg);
		    break;
#else 
	    case 'a':
	    case 's':
		    printf("-%c option not available. Use the --enable-statistics configure flag\n", o);
		    break;
#endif
	    case 'r':
		    rewr = 1;
		    break;
	    case 'w':
		    wnum = atoi(optarg);
		    if (!wnum) {
			wnum = 1;
		    }
		    break;
	    case 'b':
		    bsize = atoi(optarg);
		    break;
	    case 'u':
		    user = strdup(optarg);
		    break;
	    case 'l':
		    while(*optarg) {
			switch(*optarg) {
			    case 'A': 
				    rules |= RULES_ALL_UPPER;
				    break;
			    case 'F': 
				    rules |= RULES_FIRST_UPPER;
				    break;
			    case 'L': 
				    rules |= RULES_LAST_UPPER;
				    break;
			    case 'a': 
			    	    rules |= RULES_ALL_LOWER;
				    break;
			    case 'f': 
				    rules |= RULES_FIRST_LOWER;
				    break;
			    case 'l': 
				    rules |= RULES_LAST_LOWER;
				    break;
			    case 'D': 
				    rules |= RULES_PREPEND_DIGIT;
				    break;
			    case 'd': 
				    rules |= RULES_APPEND_DIGIT;
				    break;
			    case 'e':
				    rules |= RULES_1337;
				    break;
			    case 'x':
				    rules = RULES_ALL_UPPER
					    |	RULES_FIRST_UPPER
					    |	RULES_LAST_UPPER
					    |	RULES_ALL_LOWER
					    |	RULES_FIRST_LOWER
					    |	RULES_LAST_LOWER
					    |	RULES_PREPEND_DIGIT
					    |	RULES_APPEND_DIGIT
					    |	RULES_1337;
				    break;
			}
			optarg++;
		    }
		    break;
	    default: 
		    err_printf("unrecognized option -%c\n", o);
		    usage(*argv);
		    return 0;
	}
    }
    
    /* validate some values */
    if (optind < argc) {
	dfile = strdup(argv[optind]);
    }    
    
    if (!dfile) {
	err_printf("no wordlist file\n");
	usage(*argv);
	return 0;
    } else {
	if (strcmp(dfile, "-") == 0) {
	    read_from_stdin = 1;
	}
    }
    
    if (bsize - 1 < wnum) {
	bsize = wnum * 2;
    }
    
    /* set up signal handler */
    signal(SIGINT, sigint);
    
    /* initialize dictionary thread */
    if (dict_init(dfile, bsize, rewr) == -1) {
	err_printf("dictionary could not be initialized\n");
	return -1;
    }

    /* set rules for rewriter */
    rewr_add_rules(rules);

    /* initialize su and set up user */
    if (su_init(SU_DEFAULT_USER) == -1) {
    	err_printf("su could not be initialized\n");
	return -1;
    }
    
    if (user) {
	su_user(user);
    }

    /* initialize and spawn workers */
    if (worker_init(wnum) == -1) {
	err_printf("workers could not be initialized\n");
	return -1;
    }
    
    if (worker_spawn() == -1) {
	err_printf("workers could not be spawned\n");
	return -1;
    }

    /* main loops */
    prepare_tty(!read_from_stdin);
    
    /* prepare tty to capture keys */
    if (!read_from_stdin) {
	do {
	    o = key_pressed();
	    if (o == 'q') {
		terminate = 1;
    	    } else {
#ifdef STATISTICS
		if (ssec > 0) {
			stat_display(ssec, ansi);
		} else {
	    	    switch (o) {
	    		case -1: 
		    	    break;    
			default:
		    	    stat_display(0, ansi);
		    }
		}
#else
		if (o != -1) {
		    stat_display();
		}
#endif	
	    }
	} while(!dict_is_eof() && !terminate);	
    } else {
	do {
	    if (ssec > 0) {
		stat_display(ssec, ansi);
	    }
	} while(!dict_is_eof() && !terminate);
    }
    
    
    /* reset terminal */
    reset_tty();
    

#ifdef STATISTICS
    stat_display(0, ansi);
#endif

    /* clean up */
    dict_kill(terminate);
    worker_kill(terminate);

    printf("bye, bye...\n");

    return 0;
}
