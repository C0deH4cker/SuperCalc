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


const char* kNullErrStr             = "NULL pointer value.";
const char* kDivByZeroStr           = "Division by zero.";
const char* kModByZeroStr           = "Modulus by zero.";
const char* kVarNotFoundStr         = "No variable named '%s' found.";
const char* kBadOpTypeStr           = "Bad %s operand type: %d.";
const char* kBadCharStr             = "Unexpected character: '%c'.";
const char* kBuiltinArgsStr         = "Builtin '%s' expects %u argument%s, not %u.";
const char* kBuiltinNotFuncStr      = "Builtin '%s' is not a function.";
const char* kBadConversionStr       = "One or more arguments to builtin '%s' couldn't be converted to numbers.";
const char* kEarlyEndStr            = "Premature end of input.";
const char* kMissingPlaceholderStr  = "Missing placeholder number %z.";
const char* kBadImportDepthStr      = "Exceeded max allowed import depth when trying to import file '%s'.";
const char* kImportErrorStr         = "Failed to import file '%s': %s.";

const char* kAllocErrStr            = "Unable to allocate memory.";
const char* kBadValStr              = "Unexpected value type: %d.";
const char* kBadVarStr              = "Unexpected variable type: %d.";


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
	
	Error* ret = fmalloc(sizeof(*ret));
	ret->type = type;
	
	char* tmp;
	vasprintf(&tmp, fmt, args);
	asprintf(&ret->msg, error_messages[type], tmp);
	free(tmp);
	
	if(g_inputFileName != NULL) {
		ret->filename = strdup(g_inputFileName);
	}
	else {
		ret->filename = NULL;
	}
	
	ret->line = g_lineNumber;
	
	if(errpos != NULL) {
		ret->column = errpos - g_line + 1;
	}
	
	return ret;
}

void Error_free(Error* err) {
	if(!err) {
		return;
	}
	
	free(err->msg);
	free(err->filename);
	free(err);
}

Error* Error_copy(const Error* err) {
	if(!err) {
		return NULL;
	}
	
	Error* ret = fmalloc(sizeof(*ret));
	
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
		fprintf(stderr, "Crashing line:\n%s\n", g_line);
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
	
	UNREACHABLE();
}

