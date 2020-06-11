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

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "statement.h"
#include "defaults.h"


static void SC_registerModules(SuperCalc* sc) {
	/* Register modules */
	register_math(sc->ctx);
	register_vector(sc->ctx);
}

SuperCalc* SuperCalc_new(void) {
	SuperCalc* ret = fmalloc(sizeof(*ret));
	
	/* Create context */
	ret->ctx = Context_new();
	
	SC_registerModules(ret);
	
	return ret;
}

void SuperCalc_free(SuperCalc* sc) {
	Context_free(sc->ctx);
	free(sc);
}

void SuperCalc_run(SuperCalc* sc) {
	const char* prompt = "";
	
	if(isInteractive(stdin)) {
		sc->interactive = true;
		prompt = SC_PROMPT_NORMAL;
		
#ifdef WITH_LINENOISE
		linenoiseHistorySetMaxLen(100);
#endif
	}
	
	while((g_line = nextLine(prompt))) {
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

static Value* SC_importFile(SuperCalc* sc, const char* filename) {
	errno = 0;
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		return ValErr(importError(filename, strerror(errno)));
	}
	
	/* Save old global line buffer and swap in the new one */
	Value* ret = NULL;
	char* old_g_line = g_line;
	char* new_line = fmalloc(SC_LINE_SIZE);
	g_line = new_line;
	
	/* Save old input FILE object and swap in the new one */
	FILE* old_g_inputFile = g_inputFile;
	g_inputFile = fp;
	
	/* Evaluate each line one-by-one */
	unsigned line_number = 1;
	while(nextLine("") != NULL) {
		ret = SuperCalc_runLine(sc, new_line, V_NONE);
		if(ret != NULL && ret->type == VAL_ERR) {
			printf("%s:%u: ", filename, line_number);
			break;
		}
		
		++line_number;
	}
	
	/* Restore previous global FILE object and free the new one */
	g_inputFile = old_g_inputFile;
	fclose(fp);
	
	/* Restore previous global line buffer and free the new one */
	g_line = old_g_line;
	free(new_line);
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
				SC_registerModules(sc);
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
		
		++sc->importDepth;
		Value* ret = SC_importFile(sc, p);
		--sc->importDepth;
		
		free(code);
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
