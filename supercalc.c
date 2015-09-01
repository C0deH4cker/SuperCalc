/*
  supercalc.c
  SuperCalc

  Created by C0deH4cker on 11/22/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "supercalc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "expression.h"
#include "defaults.h"


SuperCalc* SC_new(FILE* fout) {
	SuperCalc* ret = fmalloc(sizeof(*ret));
	
	/* Create context */
	ret->ctx = Context_new();
	
	/* Register modules */
	register_math(ret->ctx);
	register_vector(ret->ctx);
	
	ret->fin = NULL;
	ret->fout = fout;
	
	return ret;
}

void SC_free(SuperCalc* sc) {
	Context_free(sc->ctx);
	free(sc);
}

Value* SC_run(const SuperCalc* sc, FILE* fp) {
	const char* prompt = "";
	if(isInteractive(fp)) {
		prompt = "sc> ";
	}
	
	Value* ret = SC_runFile(sc, fp, prompt);
	
	if(isInteractive(fp)) {
		putchar('\n');
	}
	
	return ret;
}

Value* SC_runFile(const SuperCalc* sc, FILE* fp, const char* prompt) {
	Value* ret = NULL;
	
	for(readLine(fp, prompt); !feof(fp); readLine(fp, prompt)) {
		if(ret) {
			Value_free(ret);
		}
		
		const char* p = line;
		VERBOSITY v = getVerbosity(&p);
		
		ret = SC_runString(sc, p, v);
		if(ret && ret->type != VAL_VAR) {
			Value_print(ret, sc, v);
		}
	}
	
	return ret;
}

Value* SC_runString(const SuperCalc* sc, const char* str, VERBOSITY v) {
	char* code = strdup(str);
	
	/* Strip trailing newline */
	char* end;
	if((end = strchr(code, '\n')) != NULL) *end = '\0';
	if((end = strchr(code, '\r')) != NULL) *end = '\0';
	if((end = strchr(code, '#')) != NULL) *end = '\0';
	
	const char* p = code;
	trimSpaces(&p);
	
	if(*p == '~') {
		/* Variable deletion */
		p++;
		
		char* name = nextToken(&p);
		if(name == NULL) {
			/* '~~~' means reset interpreter */
			if(p[0] == '~' && p[1] == '~') {
				/* Wipe out context */
				Context_clear(sc->ctx);
				free(code);
				return NULL;
			}
			
			if(*p == '\0') {
				free(code);
				RAISE(earlyEnd(), false);
				return NULL;
			}
			
			RAISE(badChar(*p), false);
			free(code);
			return NULL;
		}
		
		Context_del(sc->ctx, name);
		
		free(name);
		free(code);
		return NULL;
	}
	
	if(*p == '\0') {
		free(code);
		return NULL;
	}
	
	/* Parse the user's input */
	Expression* expr = Expression_parse(&p);
	free(code);
	
	/* Print expression depending on verbosity */
	Expression_print(expr, sc, v);
	
	/* Error? Go to next loop iteration */
	if(Expression_didError(expr)) {
		Expression_free(expr);
		return NULL;
	}
	
	/* Evaluate expression */
	Value* result = Expression_eval(expr, sc->ctx, v);
	Expression_free(expr);
	
	return result;
}

