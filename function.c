/*
  func.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "function.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "generic.h"
#include "context.h"


Function* Function_new(unsigned argcount, const char* const* argnames, Value* body) {
	Function* ret = fmalloc(sizeof(*ret));
	
	ret->argcount = argcount;
	
	ret->argnames = fmalloc(argcount * sizeof(*ret->argnames));
	
	/* Copy all argument names */
	unsigned i;
	for(i = 0; i < argcount; i++) {
		ret->argnames[i] = strdup(argnames[i]);
	}
	
	ret->body = body;
	
	return ret;
}

void Function_free(Function* func) {
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		free(func->argnames[i]);
	}
	
	Value_free(func->body);
	
	free(func);
}

Function* Function_copy(Function* func) {
	return Function_new(func->argcount, (const char* const*)func->argnames, Value_copy(func->body));
}

Value* Function_eval(Function* func, Context* ctx, ArgList* arglist) {
	Context* locals = Context_copy(ctx);
	
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		Value* val = Value_eval(arglist->args[i], ctx);
		Variable* arg;
		
		if(val->type == VAL_VAR) {
			Variable* var = Variable_get(ctx, val->name);
			
			switch(var->type) {
				case VAR_VALUE:
					arg = VarValue(func->argnames[i], var->val);
					break;
				
				case VAR_FUNC:
					arg = VarFunc(func->argnames[i], var->func);
					break;
				
				case VAR_BUILTIN:
					arg = VarBuiltin(func->argnames[i], var->blt);
					break;
				
				default:
					badVarType(var->type);
			}
		}
		else {
			arg = VarValue(func->argnames[i], arglist->args[i]);
		}
		
		Context_add(locals, arg);
	}
	
	return Value_eval(func->body, locals);
}

static char* argsVerbose(Function* func) {
	char* ret;
	/* 8 is big enough even for: "x, y, z", so it's a good starting size */
	size_t size = 8;
	
	ret = fmalloc((size + 1) * sizeof(*ret));
	
	ret[0] = '\0';
	
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		size_t namelen = strlen(func->argnames[i]);
		
		/* Double the string size if it's too short */
		if(strlen(ret) + namelen + 2 > size) {
			size *= 2;
			ret = frealloc(ret, (size + 1) * sizeof(*ret));
		}
		
		if(i > 0) {
			strncat(ret, ", ", size);
		}
		
		strncat(ret, func->argnames[i], size);
	}
	
	return ret;
}

char* Function_verbose(Function* func, int indent) {
	char* ret;
	
	char* current = spaces(indent);
	
	char* args = argsVerbose(func);
	
	asprintf(&ret, "(%s) {\n%s\n%s}", args,
			 Value_verbose(func->body, indent + IWIDTH),
			 current);
	
	free(current);
	free(args);
	
	return ret;
}

char* Function_repr(Function* func) {
	char* ret;
	
	char* args = argsVerbose(func);
	char* body = Value_repr(func->body);
	
	asprintf(&ret, "(%s) = %s", args, body);
	
	free(args);
	free(body);
	
	return ret;
}