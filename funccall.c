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

#include "support.h"
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
static char* reprFunc(VALTYPE valtype, const char* name, const ArgList* arglist, bool pretty);
static char* specialRepr(const char* name, const ArgList* arglist, bool pretty);
static char* verboseFunc(const char* name, const ArgList* arglist, unsigned indent);
static char* specialVerbose(const char* name, const ArgList* arglist, unsigned indent);


FuncCall* FuncCall_new(Value* func, ArgList* arglist) {
	FuncCall* ret = fmalloc(sizeof(*ret));
	
	ret->func = func;
	ret->arglist = arglist;
	
	return ret;
}

FuncCall* FuncCall_create(char* name, ArgList* arglist) {
	Value* func = ValVar(name);
	return FuncCall_new(func, arglist);
}

void FuncCall_free(FuncCall* call) {
	if(!call) {
		return;
	}
	
	Value_free(call->func);
	ArgList_free(call->arglist);
	destroy(call);
}

FuncCall* FuncCall_copy(const FuncCall* call) {
	return FuncCall_new(Value_copy(call->func), ArgList_copy(call->arglist));
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
	
	switch(var->val->type) {
		case VAL_BUILTIN:
			ret = Builtin_eval(var->val->blt, ctx, args, internal);
			
			if(!var->val->blt->isFunction) {
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
		
		case VAL_FUNC:
			ret = Function_eval(var->val->func, ctx, args);
			break;
		
		case VAL_ERR:
			/* Shouldn't be reached... */
			ret = ValErr(Error_copy(var->val->err));
			break;
		
		default:
			ret = ValErr(typeError("Variable %s is not callable", name));
			break;
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
		
		case VAL_FUNC:
			ret = Function_eval(func->func, ctx, call->arglist);
			break;
		
		case VAL_BUILTIN:
			ret = Builtin_eval(func->blt, ctx, call->arglist, false);
			break;
		
		case VAL_INT:
		case VAL_REAL:
		case VAL_FRAC:
		case VAL_VEC: {
			char* repr = Value_repr(call->func, false, false);
			ret = ValErr(typeError("Value %s is not a callable.", repr));
			destroy(repr);
			break;
		}
			
		default:
			badValType(func->type);
	}
	
	Value_free(func);
	return ret;
}

static char* reprFunc(VALTYPE valtype, const char* callable, const ArgList* arglist, bool pretty) {
	char* ret;
	char* argstr = ArgList_repr(arglist, pretty);
	const char* disp = pretty ? getPretty(callable) : callable;
	
	const char* format = "%s(%s)";
	if(valtype == VAL_FUNC) {
		format = "(%s)(%s)";
	}
	
	asprintf(&ret, format, disp, argstr);
	
	destroy(argstr);
	return ret;
}

static char* specialRepr(const char* name, const ArgList* arglist, bool pretty) {
	char* ret;
	
	if(strcmp(name, "elem") == 0) {
		if(arglist->count != 2) {
			/* Freak out */
			RAISE(internalError("Invalid argument count passed to internal call of elem"), true);
		}
		
		char* vec = Value_repr(arglist->args[0], pretty, false);
		char* index = Value_repr(arglist->args[1], pretty, false);
		
		asprintf(&ret, "%s[%s]", vec, index);
		
		destroy(index);
		destroy(vec);
	}
	else {
		/* Just default to printing the function */
		ret = reprFunc(VAL_VAR, name, arglist, pretty);
	}
	
	return ret;
}

char* FuncCall_repr(const FuncCall* call, bool pretty) {
	if(call->func->type == VAL_VAR && call->func->name[0] == '@') {
		/* Internal call */
		return specialRepr(call->func->name + 1, call->arglist, pretty);
	}
	
	char* callable = Value_repr(call->func, pretty, false);
	char* ret = reprFunc(call->func->type, callable, call->arglist, pretty);
	
	destroy(callable);
	return ret;
}

char* FuncCall_wrap(const FuncCall* call) {
	char* ret;
	char* callable = Value_wrap(call->func, false);
	char* argstr = ArgList_wrap(call->arglist);
	
	asprintf(&ret, "%s(%s)", callable, argstr);
	
	destroy(argstr);
	destroy(callable);
	return ret;
}

static char* verboseFunc(const char* name, const ArgList* arglist, unsigned indent) {
	char* ret;
	char* args = ArgList_verbose(arglist, indent + 1);
	
	asprintf(&ret,
			 "%3$s(\n"        /* name */
				 "%2$s%4$s\n" /* args */
			 "%1$s)",
			 indentation(indent), indentation(indent + 1),
			 name,
			 args);
	
	destroy(args);
	return ret;
}

static char* specialVerbose(const char* name, const ArgList* arglist, unsigned indent) {
	char* ret;
	
	if(strcmp(name, "elem") == 0) {
		if(arglist->count != 2) {
			/* Freak out */
			RAISE(internalError("Invalid argument count passed to internal call of elem"), true);
		}
		
		char* vec = Value_verbose(arglist->args[0], indent);
		char* index = Value_verbose(arglist->args[1], indent + 1);
		
		asprintf(&ret,
				 "%3$s[\n"
					 "%2$s%4$s\n"
				 "%1$s]",
				 indentation(indent), indentation(indent + 1),
				 vec, index);
		
		destroy(index);
		destroy(vec);
	}
	else {
		/* Just default to printing the function */
		ret = verboseFunc(name, arglist, indent);
	}
	
	return ret;
}

char* FuncCall_verbose(const FuncCall* call, unsigned indent) {
	if(call->func->type == VAL_VAR && call->func->name[0] == '@') {
		/* Internal call */
		return specialVerbose(&call->func->name[1], call->arglist, indent);
	}
	
	char* callable = Value_verbose(call->func, indent);
	char* ret = verboseFunc(callable, call->arglist, indent);
	
	destroy(callable);
	return ret;
}

char* FuncCall_xml(const FuncCall* call, unsigned indent) {
	/*
	 sc> ?x atan2(4, 1 + 2)
	 
	 <call>
	   <callee>
	     <var name="atan2"/>
	   </callee>
	   <args>
	     <int>4</int>
	     <add>
	       <int>1</int>
	       <int>2</int>
	     </add>
	   </args>
	 </call>
	 
	 0.927295218001612
	*/
	char* ret;
	char* callee = Value_xml(call->func, indent + 2);
	
	if(call->arglist->count > 0) {
		char* args = ArgList_xml(call->arglist, indent + 2);
		asprintf(&ret,
				 "<call>\n"
					 "%2$s<callee>\n"
						 "%3$s%4$s\n" /* func */
					 "%2$s</callee>\n"
					 "%2$s<args>\n"
						 "%5$s\n" /* args */
					 "%2$s</args>\n"
				 "%1$s</call>",
				 indentation(indent), indentation(indent + 1), indentation(indent + 2),
				 callee,
				 args);
		destroy(args);
	}
	else {
		asprintf(&ret,
				 "<call>\n"
					 "%2$s<callee>\n"
						 "%3$s%4$s\n" /* func */
					 "%2$s</callee>\n"
					 "%2$s<args/>\n"
				 "%1$s</call>",
				 indentation(indent), indentation(indent + 1), indentation(indent + 2),
				 callee);
	}
	
	destroy(callee);
	return ret;
}

