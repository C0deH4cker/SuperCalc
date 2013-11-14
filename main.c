/*
  main.c
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#include "generic.h"
#include "error.h"
#include "context.h"
#include "value.h"
#include "expression.h"
#include "defaults.h"

int main(int argc, char* argv[]) {
	Context* ctx = Context_new();
	register_math(ctx);
	
	for(nextLine(); !feof(stdin); nextLine()) {
		/* Strip trailing newline */
		char* end;
		if((end = strchr(line, '\n')) != NULL) *end = '\0';
		if((end = strchr(line, '\r')) != NULL) *end = '\0';
		
		const char* p = line;
		
		/* Get verbosity level */
		int verbose = 0;
		while(p[0] == '?') {
			verbose++;
			p++;
		}
		trimSpaces(&p);
		
		if(*p == '~') {
			/* Variable deletion */
			p++;
			
			char* name = nextToken(&p);
			if(name == NULL) {
				if(*p == '\0') {
					RAISE(earlyEnd());
					continue;
				}
				
				RAISE(badChar(*p));
				continue;
			}
			
			Context_del(ctx, name);
			
			free(name);
			
			continue;
		}
		
		/* Parse the user's input */
		Expression* expr = Expression_parse(&p);
		
		/* Print expression depending on verbosity */
		Expression_print(expr, ctx, verbose);
		
		/* Error? Go to next loop iteration */
		if(Expression_didError(expr))
			continue;
		
		/* Evaluate expression */
		Value* result = Expression_eval(expr, ctx);
		Expression_free(expr);
		
		/* Print result */
		Value_print(result, ctx);
		
		Value_free(result);
	}
	
	Context_free(ctx);
	
	return 0;
}