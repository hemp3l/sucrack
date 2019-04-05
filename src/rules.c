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
#include <string.h>
#include <ctype.h>
#include "rules.h"

char *
rules_first_upper(char *in, void *arg)
{
    char *out;

    out = strdup(in);
    out[0] = toupper(out[0]);

    return out;
}

char *
rules_last_upper(char *in, void *arg)
{
    char *out;

    out = strdup(in);
    out[strlen(out) - 1] = toupper(out[strlen(out) - 1]);

    return out;
}

char *
rules_all_upper(char *in, void *arg)
{
    char *out, *c;

    out = strdup(in);
    c = out;
    while(*c) {
		*c = toupper(*c);
		c++;
    }
    return out;
}

char *
rules_first_lower(char *in, void *arg)
{
    char *out;

    out = strdup(in);
    out[0] = tolower(out[0]);

    return out;
}

char *
rules_last_lower(char *in, void *arg)
{
    char *out;

    out = strdup(in);
    out[strlen(out) - 1] = tolower(out[strlen(out) - 1]);

    return out;
}

char *
rules_all_lower(char *in, void *arg)
{
    char *out;
    char *c;

    out = strdup(in);
    c = out;
    while(*c) {
		*c = tolower(*c);
		c++;
	}
	
    return out;
}

char *
rules_prepend_digit(char *in, void *arg)
{
    char *out;

    if (!arg) {
		return 0;
    }

    asprintf(&out, "%i%s", *(int*) arg, in);

    return out;
}

char *
rules_append_digit(char *in, void *arg)
{
    char *out;

    if (!arg) {
		return 0;
    }

    asprintf(&out, "%s%i", in, *(int*) arg);

    return out;
}

char *
rules_1337(char *in, void *arg)
{
    char *out, *c;

    out = strdup(in);

    c = out;
    while(*c) {
		switch(toupper(*c)) {
			case 'L':
				*c = '1';
				break;
			case 'E':
				*c = '3';
				break;
			case 'T':
				*c = '7';
				break;
			case 'S':
				*c = '5';
				break;
			case 'G':
				*c = '9';
				break;
			case 'O':
				*c = '0';
				break;
			case 'A':
				*c = '4';
				break;
		}
		c++;
    }
    return out;
}

