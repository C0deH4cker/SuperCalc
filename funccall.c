/*
 funccall.c
 SuperCalc
 
 Created by C0deH4cker on 11/7/13.
 Copyright (c) 2013 C0deH4cker. All rights reserved.
 */

#include "funccall.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "variable.h"
#include "arglist.h"
#include "function.h"
#include "builtin.h"
#include "binop.h"


static Value* callVar(const Context* ctx, const char* name, const ArgList* args);
static char* verboseFunc(const char* name, const ArgList* arglist, int indent);
static char* specialVerbose(const char* name, const ArgList* arglist, int indent);
static char* reprFunc(const char* name, const ArgList* arglist, bool pretty);
static char* specialRepr(const char* name, const ArgList* arglist, bool pretty);


FuncCall* FuncCall_new(Value* func, ArgList* arglist) {
	FuncCall* ret = fmalloc(sizeof(*ret));
	
	ret->func = func;
	ret->arglist = arglist;
	
	return ret;
}

FuncCall* FuncCall_create(const char* name, ArgList* arglist) {
	Value* func = ValVar(name);
	return FuncCall_new(func, arglist);
}

void FuncCall_free(FuncCall* call) {
	Value_free(call->func);
	ArgList_free(call->arglist);
	free(call);
}

FuncCall* FuncCall_copy(FuncCall* call) {
	return FuncCall_new(call->name, ArgList_copy(call->arglist));
}

static Value* callVar(const Context* ctx, const char* name, const ArgList* args) {
	Value* ret;
	
	bool internal = false;
	if(*name == '@') {
		internal = true;
		name++;
	}
	
	Variable* var = Variable_get(ctx, name);
	if(var == NULL) {
		return ValErr(varNotFound(name));
	}
	
	switch(var->type) {
		case VAR_BUILTIN:
			ret = Builtin_eval(var->blt, ctx, args, internal);
			
			if(!var->blt->isFunction) {
				if(args->count > 1) {
					Value_free(ret);
					ret = ValErr(builtinNotFunc(name));
				}
				else if(args->count == 1) {
					/* i.e. pi(2) -> pi * 2 */
					ret = ValExpr(BinOp_new(BIN_MUL, ret, Value_copy(args->args[0])));
				}
			}
			
			break;
		
		case VAR_FUNC:
			ret = Function_eval(var->func, ctx, args);
			break;
		
		case VAR_ERR:
			/* Shouldn't be reached... */
			ret = ValErr(var->err);
			break;
		
		case VAR_VALUE:
			ret = ValErr(nameError("Variable '%s' is not a function.", var->name));
			break;
		
		default:
			badVarType(var->type);
	}
	
	return ret;
}

Value* FuncCall_eval(const FuncCall* call, const Context* ctx) {
	Value* func;
	if(call->func->type == VAL_VAR) {
		func = Value_copy(call->func);
	}
	else {
		func = Value_eval(call->func, ctx);
	}
	
	if(func->type == VAL_ERR) {
		return func;
	}
	
	Value* ret;
	switch(func->type) {
		case VAL_VAR:
			ret = callVar(ctx, func->name, call->arglist);
			break;
		
		case VAL_INT:
		case VAL_REAL:
		case VAL_FRAC:
		case VAL_VEC: {
			char* repr = Value_repr(call->func, ctx);
			ret = ValErr(typeError("Value %s is not a callable.", repr));
			free(repr);
			break;
		}
			
		default:
			badValType(func->type);
	}
	
	Value_free(func);
	return ret;
}

static char* verboseFunc(const char* name, const ArgList* arglist, int indent) {
	char* ret;
	
	char* args = ArgList_verbose(arglist, indent + IWIDTH);
	char* current = spaces(indent);
	
	asprintf(&ret, "%s(\n%s%s)", name,
			 args,
			 current);
	
	free(args);
	free(current);
	
	return ret;
}

static char* specialVerbose(const char* name, const ArgList* arglist, int indent) {
	char* ret;
	
	if(strcmp(name, "abs") == 0) {
		if(arglist->count != 1) {
			return strERR();
		}
		
		char* args = ArgList_verbose(arglist, indent + IWIDTH);
		asprintf(&ret, "|%s|", args);
		free(args);
	}
	else if(strcmp(name, "elem") == 0) {
		if(arglist->count != 2) {
			/* Freak out */
			return strERR();
		}
		
		char* vec = Value_verbose(arglist->args[0], indent);
		char* index = Value_verbose(arglist->args[1], indent + IWIDTH);
		
		char* spacing = spaces(indent + IWIDTH);
		char* current = spaces(indent);
		
		asprintf(&ret, "%s[\n%s%s\n%s]", vec,
				 spacing, index,
				 current);
		
		free(vec);
		free(index);
		free(spacing);
		free(current);
	}
	else {
		/* Just default to printing the function */
		ret = verboseFunc(name, arglist, indent);
	}
	
	return ret;
}

char* FuncCall_verbose(const FuncCall* call, int indent) {
	if(call->func->type == VAL_VAR && call->func->name[0] == '@') {
		/* Internal call */
		return specialVerbose(call->func->name + 1, call->arglist, indent);
	}
	
	char* ret;
	char* callable = Value_verbose(call->func, indent);
	ret = verboseFunc(callable, call->arglist, indent);
	
	free(callable);
	
	return ret;
}

static char* reprFunc(const char* name, const ArgList* arglist, bool pretty) {
	char* ret;
	char* argstr = ArgList_repr(arglist, pretty);
	
	const char* disp = pretty ? getPretty(name) : name;
	
	asprintf(&ret, "%s(%s)", disp, argstr);
	
	free(argstr);
	
	return ret;
}

static char* specialRepr(const char* name, const ArgList* arglist, bool pretty) {
	char* ret;
	
	/* TODO: Consider removing special printing for abs in repr */
	if(strcmp(name, "abs") == 0) {
		if(arglist->count != 1) {
			/* Shouldn't ever happen */
			return strERR();
		}
		
		char* args = ArgList_repr(arglist, pretty);
		asprintf(&ret, "|%s|", args);
		free(args);
	}
	else if(strcmp(name, "elem") == 0) {
		if(arglist->count != 2) {
			/* Freak out */
			return strERR();
		}
		
		char* vec = Value_repr(arglist->args[0], pretty);
		char* index = Value_repr(arglist->args[1], pretty);
		
		asprintf(&ret, "%s[%s]", vec, index);
		
		free(vec);
		free(index);
	}
	else {
		/* Just default to printing the function */
		ret = reprFunc(name, arglist, pretty);
	}
	
	return ret;
}

char* FuncCall_repr(const FuncCall* call, bool pretty) {
	if(call->func->type == VAL_VAR && call->func->name[0] == '@') {
		/* Internal call */
		return specialRepr(call->func->name + 1, call->arglist, pretty);
	}
	
	char* ret;
	
	char* callable = Value_repr(call->func, pretty);
	ret = reprFunc(callable, call->arglist, pretty);
	
	free(callable);
	
	return ret;
}

