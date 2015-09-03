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
#include "statement.h"
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

Value* SC_run(SuperCalc* sc, FILE* fin) {
	const char* prompt = "";
	if(isInteractive(fin)) {
		sc->interactive = true;
		prompt = "sc> ";
	}
	
	Value* ret = SC_runFile(sc, fin, prompt);
	
	if(sc->interactive) {
		fputc('\n', fin);
	}
	
	return ret;
}

Value* SC_runFile(SuperCalc* sc, FILE* fin, const char* prompt) {
	Value* ret = NULL;
	const char* p;
	sc->fin = fin;
	
	while((p = readLine(sc->fout, prompt, sc->fin))) {
		if(ret) {
			Value_free(ret);
			ret = NULL;
		}
		
		VERBOSITY v = getVerbosity(&p);
		if(v & V_ERR) {
			continue;
		}
		
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
	Statement* stmt = Statement_parse(&p);
	free(code);
	
	/* Print statement depending with specified level of verbosity */
	Statement_print(stmt, sc, v);
	
	/* Error? Go to next loop iteration */
	if(Statement_didError(stmt)) {
		Statement_free(stmt);
		return NULL;
	}
	
	/* Evaluate statement */
	Value* result = Statement_eval(stmt, sc->ctx, v);
	Statement_free(stmt);
	
	return result;
}

