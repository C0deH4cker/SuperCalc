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

#ifdef WITH_LINENOISE
#include "linenoise/linenoise.h"
#endif

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "statement.h"
#include "defaults.h"


SuperCalc* SuperCalc_new(void) {
	SuperCalc* ret = fmalloc(sizeof(*ret));
	
	/* Create context */
	ret->ctx = Context_new();
	
	/* Register modules */
	register_math(ret->ctx);
	register_vector(ret->ctx);
	
	return ret;
}

void SuperCalc_free(SuperCalc* sc) {
	Context_free(sc->ctx);
	free(sc);
}

static char* SC_readLine(const char* prompt) {
#ifdef WITH_LINENOISE
	
	if(g_line != NULL) {
		free(g_line);
		g_line = NULL;
	}
	
	g_line = linenoise(prompt);
	if(g_line != NULL) {
		linenoiseHistoryAdd(g_line);
	}
	
#else /* WITH_LINENOISE */
	
	if(g_line == NULL) {
		g_line = calloc(1000, 1);
	}
	
	printf("%s", prompt);
	if(fgets(g_line, 1000, stdin) == NULL) {
		return NULL;
	}
	
#endif /* WITH_LINENOISE */
	
	return g_line;
}

void SuperCalc_run(SuperCalc* sc) {
	const char* prompt = "";
	
	if(isInteractive(stdin)) {
		sc->interactive = true;
		prompt = "sc> ";
		
#ifdef WITH_LINENOISE
		linenoiseHistorySetMaxLen(100);
#endif
	}
	
	while((g_line = SC_readLine(prompt))) {
		const char* p = g_line;
		
		VERBOSITY v = getVerbosity(&p);
		if(v & V_ERR) {
			continue;
		}
		
		Value* ret = SuperCalc_runLine(sc, p, v);
		if(ret != NULL) {
			if(ret->type != VAL_VAR) {
				Value_print(ret, sc, v);
			}
			Value_free(ret);
			ret = NULL;
		}
	}
	
	putchar('\n');
}

Value* SuperCalc_runLine(const SuperCalc* sc, const char* str, VERBOSITY v) {
	char* code = strdup(str);
	
	/* Strip trailing newline and comments */
	code = strsep(&code, "#\r\n");
	
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

