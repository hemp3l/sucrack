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

#ifndef _RULES_H
#define _RULES_H

typedef struct _rules_map_t {
    char *(*func)(char *, void *);
    int runs;
    unsigned int flag;
} rules_map_t;

#define RULES_FIRST_UPPER           (1 << 0)
#define RULES_FIRST_LOWER           (1 << 1)
#define RULES_LAST_UPPER            (1 << 2)
#define RULES_LAST_LOWER            (1 << 3)
#define RULES_ALL_UPPER             (1 << 4)
#define RULES_ALL_LOWER             (1 << 5)
#define RULES_PREPEND_DIGIT         (1 << 6)
#define RULES_APPEND_DIGIT          (1 << 7)
#define RULES_1337		    (1 << 8)

/* rule prototypes */
char    *rules_first_upper(char *in, void *arg);
char    *rules_last_upper(char *in, void *arg);
char    *rules_all_upper(char *in, void *arg);
char    *rules_first_lower(char *in, void *arg);
char    *rules_last_lower(char *in, void *arg);
char    *rules_all_lower(char *in, void *arg);
char    *rules_prepend_digit(char *in, void *arg);
char    *rules_append_digit(char *in, void *arg);
char    *rules_1337(char *in, void *arg);


static rules_map_t rules_map[] = {
    { rules_first_upper,	1,	RULES_FIRST_UPPER	},
    { rules_last_upper,		1,	RULES_LAST_UPPER	},
    { rules_all_upper, 		1,	RULES_ALL_UPPER		},
    { rules_first_lower, 	1,	RULES_FIRST_LOWER 	},
    { rules_last_lower, 	1,	RULES_LAST_LOWER 	},
    { rules_all_lower, 		1,	RULES_ALL_LOWER 	},
    { rules_prepend_digit,	10,	RULES_PREPEND_DIGIT	},
    { rules_append_digit,	10,	RULES_APPEND_DIGIT	},
    { rules_1337,		8,	RULES_1337		}
};


#endif

