/*
  variable.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "variable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "generic.h"
#include "error.h"
#include "value.h"
#include "context.h"
#include "function.h"
#include "builtin.h"
#include "value.h"


static Variable* allocVar(VARTYPE type, const char* name) {
	Variable* ret = fmalloc(sizeof(*ret));
	
	memset(ret, 0, sizeof(*ret));
	
	ret->type = type;
	ret->name = name ? strdup(name) : NULL;
	
	return ret;
}

Variable* VarErr(Error* err) {
	Variable* ret = allocVar(VAR_ERR, "error");
	
	ret->err = err;
	
	return ret;
}

Variable* VarBuiltin(const char* name, Builtin* blt) {
	Variable* ret = allocVar(VAR_BUILTIN, name);
	
	ret->blt = blt;
	
	return ret;
}

Variable* VarValue(const char* name, Value* val) {
	Variable* ret = allocVar(VAR_VALUE, name);
	
	ret->val = val;
	
	return ret;
}

Variable* VarFunc(const char* name, Function* func) {
	Variable* ret = allocVar(VAR_FUNC, name);
	
	ret->func = func;
	
	return ret;
}

void Variable_free(Variable* var) {
	if(!var) return;
	
	switch(var->type) {
		case VAR_BUILTIN:
			Builtin_free(var->blt);
			break;
		
		case VAR_VALUE:
			Value_free(var->val);
			break;
		
		case VAR_FUNC:
			Function_free(var->func);
			break;
		
		case VAR_ERR:
			Error_free(var->err);
			break;
		
		default:
			badVarType(var->type);
	}
	
	free(var->name);
	free(var);
}

Variable* Variable_copy(Variable* var) {
	Variable* ret;
	
	switch(var->type) {
		case VAR_BUILTIN:
			ret = VarBuiltin(var->name, Builtin_copy(var->blt));
			break;
		
		case VAR_VALUE:
			ret = VarValue(var->name, Value_copy(var->val));
			break;
		
		case VAR_FUNC:
			ret = VarFunc(var->name, Function_copy(var->func));
			break;
		
		case VAR_ERR:
			ret = VarErr(Error_copy(var->err));
			break;
		
		default:
			badVarType(var->type);
	}
	
	return ret;
}

Value* Variable_eval(const char* name, Context* ctx) {
	Value* ret;
	
	/* Look up variable */
	Variable* var = Variable_get(ctx, name);
	if(var == NULL) {
		ret = ValErr(varNotFound(name));
	}
	else if(var->type == VAR_FUNC) {
		ret = ValVar(var->name);
	}
	else if(var->type == VAR_BUILTIN) {
		ArgList* noArgs = ArgList_new(0);
		ret = Builtin_eval(var->blt, ctx, noArgs);
		ArgList_free(noArgs);
	}
	else {
		ret = Value_copy(var->val);
	}
	
	return ret;
}

Variable* Variable_get(Context* ctx, const char* name) {
	return Context_get(ctx, name);
}

Variable* Variable_getAbove(Context* ctx, const char* name) {
	return Context_getAbove(ctx, name);
}

void Variable_update(Variable* dst, Variable* src) {
	/* Free old value */
	switch(dst->type) {
		case VAR_BUILTIN:
			Builtin_free(dst->blt);
			break;
		
		case VAR_ERR:
			Error_free(dst->err);
			break;
		
		case VAR_FUNC:
			Function_free(dst->func);
			break;
		
		case VAR_VALUE:
			Value_free(dst->val);
			break;
		
		default:
			badVarType(dst->type);
	}
	
	switch(src->type) {
		case VAR_BUILTIN:
			dst->blt = src->blt;
			src->blt = NULL;
			break;
		
		case VAR_ERR:
			dst->err = src->err;
			src->err = NULL;
			break;
		
		case VAR_FUNC:
			dst->func = src->func;
			src->func = NULL;
			break;
		
		case VAR_VALUE:
			dst->val = src->val;
			src->val = NULL;
			break;
		
		default:
			badVarType(src->type);
	}
	
	dst->type = src->type;
	
	free(src);
}

char* Variable_verbose(Variable* var) {
	char* ret;
	
	if(var->type == VAR_FUNC) {
		char* func = Function_verbose(var->func);
		asprintf(&ret, "%s%s", var->name, func);
		free(func);
	}
	else if(var->type == VAR_BUILTIN) {
		char* blt = Builtin_verbose(var->blt, 0);
		if(var->name == NULL) {
			ret = blt;
		}
		else {
			asprintf(&ret, "%s = %s", var->name, blt);
			free(blt);
		}
	}
	else {
		char* val = Value_verbose(var->val, 0);
		if(var->name == NULL) {
			ret = val;
		}
		else {
			asprintf(&ret, "%s = %s", var->name, val);
			free(val);
		}
	}
	
	return ret;
}

char* Variable_repr(Variable* var) {
	char* ret;
	
	if(var->type == VAR_FUNC) {
		char* func = Function_repr(var->func);
		asprintf(&ret, "%s%s", var->name, func);
		free(func);
	}
	else if(var->type == VAR_BUILTIN) {
		char* blt = Builtin_repr(var->blt);
		if(var->name == NULL) {
			ret = blt;
		}
		else {
			asprintf(&ret, "%s = %s", var->name, blt);
			free(blt);
		}
	}
	else {
		char* val = Value_repr(var->val);
		if(var->name == NULL) {
			ret = val;
		}
		else {
			asprintf(&ret, "%s = %s", var->name, val);
			free(val);
		}
	}
	
	return ret;
}

