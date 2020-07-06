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


DEF(SuperCalc);

SuperCalc* SuperCalc_new(void) {
	SuperCalc* ret = OBJECT_ALLOC(SuperCalc);
	
	/* Create context */
	ret->ctx = Context_new();
	
	SC_registerModules(ret);
	
	return ret;
}

void SuperCalc_free(SuperCalc* sc) {
	if(!sc) {
		return;
	}
	
	Context_free(sc->ctx);
	OBJECT_FREE(SuperCalc, sc);
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
		char* p = g_line;
		
		/* I solemnly swear not to modify the string's contents. Pinky promise :) */
		VERBOSITY v = getVerbosity((istring)&p);
		if(v & V_ERR) {
			continue;
		}
		
		Value* ret = SuperCalc_runLine(sc, p, v);
		if(ret != NULL) {
			if(ret->type != VAL_VAR) {
				Value_print(ret, v);
			}
			Value_free(ret);
			ret = NULL;
		}
	}
	
	putchar('\n');
}

Error* SuperCalc_importFile(SuperCalc* sc, const char* filename) {
	errno = 0;
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		return importError(filename, strerror(errno));
	}
	
	/* Save old globals and swap in the new ones */
	char* old_g_line = g_line;
	unsigned old_g_lineNumber = g_lineNumber;
	FILE* old_g_inputFile = g_inputFile;
	const char* old_g_inputFileName = g_inputFileName;
	
	char* new_line = fmalloc(SC_LINE_SIZE);
	g_line = new_line;
	g_lineNumber = 0;
	g_inputFile = fp;
	g_inputFileName = filename;
	
	Error* ret = NULL;
	
	/* Evaluate each line one-by-one */
	while(nextLine("") != NULL) {
		Value* val = SuperCalc_runLine(sc, new_line, V_NONE);
		if(val != NULL && val->type == VAL_ERR) {
			ret = val->err;
			val->err = CAST_NONNULL(NULL);
			Value_free(val);
			break;
		}
		
		Value_free(val);
	}
	
	/* Restore previous global FILE object and free the new one */
	g_inputFile = old_g_inputFile;
	fclose(fp);
	
	/* Restore previous global line buffer and free the new one */
	g_line = old_g_line;
	destroy(new_line);
	g_lineNumber = old_g_lineNumber;
	g_inputFileName = old_g_inputFileName;
	
	return ret;
}

Value* SuperCalc_runLine(SuperCalc* sc, char* code, VERBOSITY v) {
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
				return NULL;
			}
			
			if(*p == '\0') {
				return ValErr(earlyEnd(p));
			}
			
			Value* err = ValErr(badChar(p));
			return err;
		}
		
		Context_del(sc->ctx, name);
		
		destroy(name);
		return NULL;
	}
	else if(*p == '@') {
		/* File import */
		p++;
		
		if(sc->importDepth > 9) {
			Value* err = ValErr(badImportDepth(p));
			return err;
		}
		
		++sc->importDepth;
		Error* ret = SuperCalc_importFile(sc, p);
		--sc->importDepth;
		
		if(ret != NULL) {
			return ValErr(ret);
		}
		return NULL;
	}
	else if(*p == '\0') {
		return NULL;
	}
	
	/* Parse the user's input */
	Statement* stmt = Statement_parse(&p);
	
	/* Print statement depending with specified level of verbosity */
	Statement_print(stmt, sc, v);
	
	/* Error? Go to next loop iteration */
	if(Statement_didError(stmt)) {
		Value* ret = stmt->var->val;
		stmt->var->val = CAST_NONNULL(NULL);
		Statement_free(stmt);
		return ret;
	}
	
	/* Evaluate statement */
	Value* result = Statement_eval(stmt, sc->ctx, v);
	Statement_free(stmt);
	
	return result;
}

METHOD_debugString(SuperCalc) {
	UNREFERENCED_PARAMETER(self);
	return strdup("SuperCalc");
}
