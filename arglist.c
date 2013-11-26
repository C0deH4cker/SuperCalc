/*
  arglist.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "arglist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "generic.h"
#include "value.h"
#include "context.h"


ArgList* ArgList_new(unsigned count) {
	ArgList* ret = fmalloc(sizeof(*ret));
	
	size_t len = count * sizeof(*ret->args);
	
	ret->count = count;
	ret->args = fmalloc(len);
	
	memset(ret->args, 0, len);
	
	return ret;
}

void ArgList_free(ArgList* arglist) {
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value_free(arglist->args[i]);
	}
	
	free(arglist);
}

ArgList* ArgList_create(unsigned count, ...) {
	va_list va;
	va_start(va, count);
	
	ArgList* ret = ArgList_vcreate(count, va);
	
	va_end(va);
	
	return ret;
}

ArgList* ArgList_vcreate(unsigned count, va_list args) {
	ArgList* ret = ArgList_new(count);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		Value* val = va_arg(args, Value*);
		ret->args[i] = val;
	}
	
	return ret;
}

ArgList* ArgList_copy(ArgList* arglist) {
	unsigned count = arglist->count;
	ArgList* ret = ArgList_new(count);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		ret->args[i] = Value_copy(arglist->args[i]);
	}
	
	return ret;
}

ArgList* ArgList_eval(ArgList* arglist, Context* ctx) {
	ArgList* ret = ArgList_new(arglist->count);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value* result = Value_coerce(arglist->args[i], ctx);
		if(result->type == VAL_ERR) {
			/* An error occurred */
			Error_raise(result->err);
			
			Value_free(result);
			ArgList_free(ret);
			
			return NULL;
		}
		
		ret->args[i] = result;
	}
	
	return ret;
}

double* ArgList_toReals(ArgList* arglist, Context* ctx) {
	double* ret = fmalloc(arglist->count * sizeof(*ret));
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		double real = Value_asReal(arglist->args[i]);
		if(isnan(real)) {
			free(ret);
			return NULL;
		}
		
		ret[i] = real;
	}
	
	return ret;
}

ArgList* ArgList_parse(const char** expr, char sep, char end) {
	/* Since most funcs take at most 2 args, 2 is a good starting size */
	unsigned size = 2;
	unsigned count = 0;
	
	Value** args = fmalloc(size * sizeof(*args));
	
	Value* arg = Value_parse(expr, sep, end);
	trimSpaces(expr);
	
	while(arg->type != VAL_END && arg->type != VAL_ERR) {
		if(count >= size) {
			size *= 2;
			args = frealloc(args, size * sizeof(*args));
		}
		
		args[count++] = arg;
		
		trimSpaces(expr);
		if(**expr != sep)
			break;
		
		(*expr)++;
		
		arg = Value_parse(expr, sep, end);
	}
	
	if(**expr && **expr != end) {
		/* Not NUL and not end means invalid char */
		unsigned i;
		for(i = 0; i < count; i++) {
			free(args[i]);
		}
		free(args);
		
		RAISE(badChar(**expr));
		return NULL;
	}
	
	ArgList* ret = ArgList_new(count);
	memcpy(ret->args, args, count * sizeof(*args));
	
	free(args);
	
	if(**expr == end)
		(*expr)++;
	
	return ret;
}

char* ArgList_verbose(ArgList* arglist, int indent) {
	char* ret;
	
	size_t size = 32;
	
	ret = fmalloc(size + 1);
	
	ret[0] = '\0';
	
	char* spacing = spaces(indent);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value* arg = arglist->args[i];
		char* argstr = Value_verbose(arg, indent);
		char* curarg;
		
		asprintf(&curarg, "%s[%d] %s\n", spacing, i, argstr);
		
		free(argstr);
		
		/* Double the string size if it's too short */
		size_t newlen = strlen(ret) + strlen(curarg);
		if(newlen > size) {
			size = newlen;
			ret = frealloc(ret, (size + 1) * sizeof(*ret));
		}
		
		strncat(ret, curarg, size);
		
		free(curarg);
	}
	
	free(spacing);
	
	return ret;
}

char* ArgList_repr(ArgList* arglist) {
	char* ret;
	
	size_t size = 32;
	
	ret = fmalloc(size + 1);
	
	ret[0] = '\0';
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value* arg = arglist->args[i];
		char* argstr = Value_repr(arg);
		
		/* Double the string size if it's too short */
		size_t newlen = strlen(ret) + 2 + strlen(argstr);
		if(newlen > size) {
			size = newlen;
			ret = frealloc(ret, (size + 1) * sizeof(*ret));
		}
		
		if(i > 0) {
			strncat(ret, ", ", size);
		}
		
		strncat(ret, argstr, size);
		
		free(argstr);
	}
	
	return ret;
}

