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

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "variable.h"
#include "arglist.h"
#include "function.h"


FuncCall* FuncCall_new(const char* name, ArgList* arglist) {
	FuncCall* ret = fmalloc(sizeof(*ret));
	
	ret->name = strdup(name);
	ret->arglist = arglist;
	
	return ret;
}

void FuncCall_free(FuncCall* call) {
	free(call->name);
	ArgList_free(call->arglist);
}

FuncCall* FuncCall_copy(FuncCall* call) {
	return FuncCall_new(strdup(call->name), ArgList_copy(call->arglist));
}

Value* FuncCall_eval(FuncCall* call, Context* ctx) {
	Value* ret;
	Variable* var = Variable_get(ctx, call->name);
	
	if(var == NULL) {
		return ValErr(varNotFound(call->name));
	}
	
	if(var->type == VAR_FUNC) {
		/* Call the function */
		ret = Function_eval(var->func, ctx, call->arglist);
	}
	else if(var->type == VAR_BUILTIN) {
		ret = Builtin_eval(var->blt, ctx, call->arglist);
	}
	else {
		/* Handle cases like a(4 + 6/7) -> a * (4 + 6/7) */
		if(call->arglist->count != 1) {
			ret = ValErr(nameError("Variable '%s' is not a function.", call->name));
		}
		else {
			/* Just multiply the variable by the value in the parentheses */
			BinOp* mul = BinOp_new(BIN_MUL, Value_copy(var->val), Value_copy(call->arglist->args[0]));
			ret = BinOp_eval(mul, ctx);
			BinOp_free(mul);
		}
	}
	
	return ret;
}

char* FuncCall_verbose(FuncCall* call, int indent) {
	char* ret;
	
	char* argstr = ArgList_verbose(call->arglist, indent + IWIDTH);
	char* current = spaces(indent);
	
	asprintf(&ret, "%s(\n%s%s)", call->name, argstr, current);
	
	free(argstr);
	free(current);
	
	return ret;
}

char* FuncCall_repr(FuncCall* call) {
	char* ret;
	
	char* argstr = ArgList_repr(call->arglist);
	
	asprintf(&ret, "%s(%s)", call->name, argstr);
	
	free(argstr);
	
	return ret;
}