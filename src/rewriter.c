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
#include <string.h>
#include <ctype.h>
#include "rewriter.h"
#include "util.h"
#include "rules.h"
#include "stat.h"

static unsigned int rules;
static int rnum;

#ifdef STATISTICS
static stat_rewr_t rstat;
#endif

extern rules_map_t rules_map[];

void
rewr_init()
{
    rules = 0x0000;
    rnum = 0;
}
      
void
rewr_add_rules(unsigned int r)
{
    int s;
    
    rules |= r;

#ifdef STATISTICS    
    for (s = 0; s < sizeof(int) * 8; s++) {
	rstat.rnum += (r >> s) & 0x1;
    }
    
    rstat.rules = rules;
#endif

}

rule_set_t *
rewr_get_rules()
{
    rule_set_t *rule_set;

    rule_set = (rule_set_t *) malloc(sizeof(rule_set_t));
    rule_set->rules = rules;
    rule_set->remaining_runs = 0;
    rule_set->current_rule = -1;

    return rule_set;
}

char *
rewr_rewrite(char *word, rule_set_t *rule_set)
{
    int r;
	// if a rule is done
	if (!rule_set->remaining_runs && rule_set->current_rule != -1) {
	    rule_set->rules &= ~rules_map[rule_set->current_rule].flag;
	    rule_set->current_rule = -1;
	}

    if (rule_set->rules) {

	// find next rule
	if (rule_set->current_rule == -1) {
	    r = 0;
	    while (!(rule_set->rules & rules_map[r].flag)) {
		r++;
	    }
	    rule_set->current_rule = r;
	    rule_set->remaining_runs = rules_map[r].runs;
	}

	debug_printf("rewr: rule = %i (%i) runs = %i\n", rule_set->current_rule, rules_map[rule_set->current_rule].flag, rule_set->remaining_runs);
	// apply rule
	rule_set->remaining_runs--;
	return rules_map[rule_set->current_rule].func(word, &(rule_set->remaining_runs));
    }


    return 0;
}

#ifdef STATISTICS
void 
rewr_get_stats(stat_rewr_t **stats)
{
    *stats = &rstat;
}
#endif
