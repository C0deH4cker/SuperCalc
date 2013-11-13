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


static char line[1024];
static char* nextLine(void) {
	fprintf(stderr, ">>> ");
	fgets(line, sizeof(line), stdin);
	
	return line;
}

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
		
		/* Parse the user's input */
		Expression* expr = Expression_parse(&p);
		
		/* Error parsing? */
		if(expr->var->type == VAR_ERR) {
			Error_raise(expr->var->err);
			Expression_free(expr);
			continue;
		}
		
		if(verbose >= 2) {
			/* Dump expression tree */
			char* tree = Expression_verbose(expr, 0);
			fprintf(stderr, "Dumping parse tree:\n");
			printf("%s\n", tree);
			free(tree);
		}
		
		if(verbose >= 1) {
			/* Print parenthesized expression */
			char* reprinted = Expression_repr(expr);
			printf("%s = ", reprinted);
			free(reprinted);
		}
		
		/* Evaluate expression */
		Value* result = Expression_eval(expr, ctx);
		Expression_free(expr);
		
		if(result->type == VAL_ERR) {
			/* An error occurred, so print it and continue. */
			Error_raise(result->err);
			Value_free(result);
			continue;
		}
		
		/* Print the result of the expression */
		char* resultString = Value_repr(result);
		if(resultString) {
			printf("%s", resultString);
			free(resultString);
		}
		
		/* If the result is a fraction, also print out the floating point representation */
		if(result->type == VAL_FRAC) {
			printf(" (%.*g)", DBL_DIG, Fraction_asReal(result->frac));
		}
		
		putchar('\n');
		
		Value_free(result);
	}
	
	Context_free(ctx);
	
	return 0;
}