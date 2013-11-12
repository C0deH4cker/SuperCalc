/*
  error.c
  SuperCalc

  Created by C0deH4cker on 11/10/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "generic.h"



const char* kNullErrStr		   = "NULL pointer value.";
const char* kDivByZeroStr	   = "Division by zero.";
const char* kVarNotFoundStr	   = "No variable named '%s' found.";
const char* kBadOpTypeStr	   = "Bad %s operand type: %d.";
const char* kBadCharStr		   = "Unexpected character: '%c'.";
const char* kBuiltinArgsStr    = "Builtin '%s' takes %u arguments, not %u.";
const char* kBuiltinNotFuncStr = "Builtin '%s' is not a function.";
const char* kAllocErrStr	   = "Unable to allocate memory.";
const char* kBadValStr		   = "Unexpected value type: %d.";
const char* kBadVarStr		   = "Unexpected variable type: %d.";


static const char* error_messages[] = {
	"Math Error: %s\n",
	"Syntax Error: %s\n",
	"Fatal Error: %s\n",
	"Name Error: %s\n",
	"Type Error: %s\n",
	"Unknown Error: %s\n",
	""
};


Error* Error_new(ERRTYPE type, const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	
	Error* ret = Error_vnew(type, fmt, va);
	
	va_end(va);
	
	return ret;
}

Error* Error_vnew(ERRTYPE type, const char* fmt, va_list args) {
	Error* ret = fmalloc(sizeof(*ret));
	ret->type = type;
	
	char* tmp;
	vasprintf(&tmp, fmt, args);
	
	asprintf(&ret->msg, error_messages[type], tmp);
	
	free(tmp);
	
	return ret;
}

void Error_free(Error* err) {
	free(err->msg);
	free(err);
}

Error* Error_copy(Error* err) {
	Error* ret = fmalloc(sizeof(*ret));
	
	ret->type = err->type;
	ret->msg = strdup(err->msg);
	
	return ret;
}

void Error_raise(Error* err) {
	fprintf(stderr, "%s", err->msg);
	
	if(!Error_canRecover(err))
		exit(EXIT_FAILURE);
}

bool Error_canRecover(Error* err) {
	switch(err->type) {
		case ERR_MATH:
		case ERR_NAME:
		case ERR_SYNTAX:
		case ERR_TYPE:
		case ERR_IGN:
			return true;
		
		case ERR_FATAL:
		case ERR_UNK:
		default:
			return false;
	}
}

void die(const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	
	Error* err = Error_vnew(ERR_FATAL, fmt, va);
	
	va_end(va);
	
	Error_raise(err);
	/* Error_raise will cause the program to die */
	
	__builtin_unreachable();
}


