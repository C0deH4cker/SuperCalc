/*
  error.c
  SuperCalc

  Created by C0deH4cker on 11/10/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "support.h"
#include "generic.h"


static const char* error_messages[] = {
	"",
	"Math Error: %s\n",
	"Syntax Error: %s\n",
	"Fatal Error: %s\n",
	"Name Error: %s\n",
	"Type Error: %s\n",
	"Runtime Error: %s\n",
	"Internal Error: %s\n",
	"Unknown Error: %s\n"
};


DEF(Error);

Error* Error_new(ERRTYPE type, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	Error* ret = Error_vnew(type, fmt, args);
	
	va_end(args);
	
	return ret;
}

Error* Error_vnew(ERRTYPE type, const char* fmt, va_list args) {
	const char* errpos = NULL;
	
	if(type == ERR_SYNTAX) {
		errpos = fmt;
		fmt = va_arg(args, const char*);
	}
	
	Error* ret = OBJECT_ALLOC(Error);
	ret->type = type;
	
	char* tmp;
	vasprintf(&tmp, fmt, args);
	asprintf(&ret->msg, error_messages[type], tmp);
	destroy(tmp);
	
	if(g_inputFileName != NULL) {
		ret->filename = strdup(g_inputFileName);
	}
	else {
		ret->filename = NULL;
	}
	
	ret->line = g_lineNumber;
	
	if(errpos != NULL) {
		ret->column = (unsigned)(errpos - g_line) + 1;
	}
	
	return ret;
}

void Error_free(Error* err) {
	if(!err) {
		return;
	}
	
	destroy(err->msg);
	destroy(err->filename);
	OBJECT_FREE(Error, err);
}

Error* Error_copy(const Error* err) {
	Error* ret = OBJECT_ALLOC(Error);
	
	ret->type = err->type;
	ret->msg = strdup(err->msg);
	
	if(err->filename != NULL) {
		ret->filename = strdup(err->filename);
	}
	ret->line = err->line;
	ret->column = err->column;
	
	return ret;
}

void Error_raise(const Error* err, bool forceDeath) {
	if(err->filename != NULL) {
		fprintf(stderr, "%s:", err->filename);
		if(err->line > 0) {
			fprintf(stderr, "%u:", err->line);
			if(err->column > 0) {
				fprintf(stderr, "%u:", err->column);
			}
		}
		
		fprintf(stderr, " ");
	}
	
	fprintf(stderr, "%s", err->msg);
	
	if(forceDeath || !Error_canRecover(err)) {
		/* Useful to set a breakpoint on the next line for debugging */
		fprintf(stderr, "Crashing line:\n%s", g_line);
		abort();
	}
}

bool Error_canRecover(const Error* err) {
	switch(err->type) {
		case ERR_MATH:
		case ERR_NAME:
		case ERR_SYNTAX:
		case ERR_TYPE:
		case ERR_IGN:
		case ERR_RUNTIME:
			return true;
		
		case ERR_FATAL:
		case ERR_INTERNAL:
		case ERR_UNK:
		default:
			return false;
	}
}

void die(const char* file, const char* function, int line, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	Error* err = Error_vnew(ERR_FATAL, fmt, args);
	
	va_end(args);
	
	fprintf(stderr, "\n\nFile %s in %s on line %d:\n", file, function, line);
	Error_raise(err, true);
	/* Error_raise will cause the program to die */
	
	UNREACHABLE;
}

METHOD_debugString(Error) {
	char* ret = NULL;
	asprintf(&ret, "Error{%d, %s}", self->type, self->msg);
	return ret;
}
