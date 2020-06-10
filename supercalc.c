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
#include <errno.h>

#ifdef WITH_LINENOISE
#include "linenoise/linenoise.h"
#endif

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "statement.h"
#include "defaults.h"


#define SC_LINE_SIZE 1000


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
		g_line = fmalloc(SC_LINE_SIZE);
	}
	
	printf("%s", prompt);
	if(fgets(g_line, SC_LINE_SIZE, stdin) == NULL) {
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

static Value* SC_importFile(SuperCalc* sc, FILE* fp) {
	Value* ret = NULL;
	char* old_g_line = g_line;
	char* new_line = fmalloc(SC_LINE_SIZE);
	g_line = new_line;
	
	while(fgets(new_line, SC_LINE_SIZE, fp) != NULL) {
		ret = SuperCalc_runLine(sc, new_line, V_NONE);
		if(ret != NULL && ret->type == VAL_ERR) {
			break;
		}
	}
	
	g_line = old_g_line;
	return ret;
}

Value* SuperCalc_runLine(SuperCalc* sc, const char* str, VERBOSITY v) {
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
				return ValErr(earlyEnd());
			}
			
			Value* err = ValErr(badChar(*p));
			free(code);
			return err;
		}
		
		Context_del(sc->ctx, name);
		
		free(name);
		free(code);
		return NULL;
	}
	else if(*p == '@') {
		/* File import */
		p++;
		
		if(sc->importDepth > 9) {
			Value* err = ValErr(badImportDepth(p));
			free(code);
			return err;
		}
		
		errno = 0;
		FILE* fp = fopen(p, "r");
		if(fp == NULL) {
			Value* err = ValErr(importError(p, strerror(errno)));
			free(code);
			return err;
		}
		
		++sc->importDepth;
		Value* ret = SC_importFile(sc, fp);
		--sc->importDepth;
		return ret;
	}
	else if(*p == '\0') {
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
