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
#include <stddef.h>
#include <math.h>

#include "support.h"
#include "generic.h"
#include "error.h"
#include "value.h"
#include "context.h"


static char* arglistToString(const ArgList* arglist, char** argstrs);


ArgList* ArgList_new(unsigned count) {
	ArgList* ret = fmalloc(sizeof(*ret));
	
	ret->count = count;
	ret->args = count != 0 ? fcalloc(count, sizeof(*ret->args)) : NULL;
	
	return ret;
}

void ArgList_free(ArgList* arglist) {
	if(!arglist) {
		return;
	}
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value_free(arglist->args[i]);
	}
	
	free(arglist->args);
	free(arglist);
}

ArgList* ArgList_create(unsigned count, ...) {
	va_list args;
	va_start(args, count);
	
	ArgList* ret = ArgList_vcreate(count, args);
	
	va_end(args);
	
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

ArgList* ArgList_copy(const ArgList* arglist) {
	if(!arglist) {
		return NULL;
	}
	
	unsigned count = arglist->count;
	ArgList* ret = ArgList_new(count);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		ret->args[i] = Value_copy(arglist->args[i]);
	}
	
	return ret;
}

void ArgList_setScope(ArgList* arglist, const Context* ctx) {
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value_setScope(arglist->args[i], ctx);
	}
}

ArgList* ArgList_eval(const ArgList* arglist, const Context* ctx) {
	ArgList* ret = ArgList_new(arglist->count);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value* result = Value_eval(arglist->args[i], ctx);
		if(result->type == VAL_ERR) {
			/* An error occurred */
			Error_raise(result->err, false);
			
			Value_free(result);
			ArgList_free(ret);
			
			return NULL;
		}
		
		ret->args[i] = result;
	}
	
	return ret;
}

double* ArgList_toReals(const ArgList* arglist, const Context* ctx) {
	UNREFERENCED_PARAMETER(ctx);
	
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

ArgList* ArgList_parse(const char** expr, char sep, char end, parser_cb* cb, Error** err) {
	/* Since most funcs take at most 2 args, 2 is a good starting size */
	unsigned size = 2;
	unsigned count = 0;
	unsigned i;
	
	*err = NULL;
	
	Value** args = fmalloc(size * sizeof(*args));
	
	Value* arg = Value_parse(expr, sep, end, cb);
	trimSpaces(expr);
	
	while(arg->type != VAL_END && arg->type != VAL_ERR) {
		if(count >= size) {
			size *= 2;
			args = frealloc(args, size * sizeof(*args));
		}
		
		args[count++] = arg;
		arg = NULL;
		
		trimSpaces(expr);
		if(**expr != sep) {
			break;
		}
		
		(*expr)++;
		
		arg = Value_parse(expr, sep, end, cb);
	}
	
	if(arg && arg->type == VAL_ERR) {
		for(i = 0; i < count; i++) {
			Value_free(args[i]);
		}
		free(args);
		
		*err = arg->err;
		arg->err = NULL;
		Value_free(arg);
		return NULL;
	}
	
	if(arg) {
		Value_free(arg);
	}
	
	if(**expr && **expr != end) {
		/* Not NUL and not end means invalid char */
		for(i = 0; i < count; i++) {
			Value_free(args[i]);
		}
		free(args);
		
		*err = badChar(*expr);
		return NULL;
	}
	
	ArgList* ret = ArgList_new(count);
	memcpy(ret->args, args, count * sizeof(*args));
	free(args);
	
	if(**expr == end) {
		(*expr)++;
	}
	
	return ret;
}

static char* arglistToString(const ArgList* arglist, char** argstrs) {
	char* ret;
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		if(i == 0) {
			ret = argstrs[i];
		}
		else {
			char* tmp;
			asprintf(&tmp,
					 "%s, %s",
					 ret, argstrs[i]);
			free(ret);
			free(argstrs[i]);
			ret = tmp;
		}
	}
	
	free(argstrs);
	return ret;
}

char* ArgList_repr(const ArgList* arglist, bool pretty) {
	if(arglist->count == 0) {
		return strdup("");
	}
	
	char** argstrs = fmalloc(arglist->count * sizeof(*argstrs));
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		argstrs[i] = Value_repr(arglist->args[i], pretty, false);
	}
	
	return arglistToString(arglist, argstrs);
}

char* ArgList_wrap(const ArgList* arglist) {
	if(arglist->count == 0) {
		return strdup("");
	}
	
	char** argstrs = fmalloc(arglist->count * sizeof(*argstrs));
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		argstrs[i] = Value_wrap(arglist->args[i], false);
	}
	
	return arglistToString(arglist, argstrs);
}

char* ArgList_verbose(const ArgList* arglist, unsigned indent) {
	char* ret;
	const char* current = indentation(indent);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		char* argstr = Value_verbose(arglist->args[i], indent);
		
		if(i == 0) {
			asprintf(&ret,
					 "[%d] %s",
					 i, argstr);
		}
		else {
			char* tmp;
			asprintf(&tmp,
					 "%s\n"
					 "%s[%d] %s",
					 ret,
					 current, i, argstr);
			free(ret);
			ret = tmp;
		}
		
		free(argstr);
	}
	
	return ret;
}

char* ArgList_xml(const ArgList* arglist, unsigned indent) {
	/*
	 sc> ?x logbase(22/7, 0.5pi)
	 
	 <call>
	   <callee>
	     <var name="logbase"/>
	   </callee>
	   <args>
	     <div>
	       <int>22</int>
	       <int>7</int>
	     </div>
	     <mul>
	       <real>0.5</real>
	       <var name="pi"/>
	     </mul>
	   </args>
	 </call>
	*/
	if(arglist->count == 0) {
		return NULL;
	}
	
	char* ret;
	const char* spacing = indentation(indent);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		char* arg = Value_xml(arglist->args[i], indent);
		
		if(i == 0) {
			asprintf(&ret,
					 "%s%s", /* first arg */
					 spacing, arg);
		}
		else {
			char* tmp;
			asprintf(&tmp,
					 "%s\n"  /* preceding args */
					 "%s%s", /* current arg */
					 ret,
					 spacing, arg);
			free(ret);
			ret = tmp;
		}
		
		free(arg);
	}
	
	return ret;
}

