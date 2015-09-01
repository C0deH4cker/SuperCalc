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
#include "variable.h"
#include "arglist.h"


static Variable* allocVar(VARTYPE type, char* name) {
	Variable* ret = fcalloc(1, sizeof(*ret));
	
	ret->type = type;
	ret->name = name;
	
	return ret;
}

Variable* VarErr(Error* err) {
	Variable* ret = allocVar(VAR_ERR, strdup("error"));
	ret->err = err;
	return ret;
}

Variable* VarBuiltin(char* name, Builtin* blt) {
	Variable* ret = allocVar(VAR_BUILTIN, name);
	ret->blt = blt;
	return ret;
}

Variable* VarConstant(char* name, Builtin* blt) {
	Variable* ret = allocVar(VAR_CONSTANT, name);
	ret->blt = blt;
	return ret;
}

Variable* VarValue(char* name, Value* val) {
	Variable* ret = allocVar(VAR_VALUE, name);
	ret->val = val;
	return ret;
}

Variable* VarFunc(char* name, Function* func) {
	Variable* ret = allocVar(VAR_FUNC, name);
	ret->func = func;
	return ret;
}

void Variable_free(Variable* var) {
	if(!var) {
		return;
	}
	
	switch(var->type) {
		case VAR_BUILTIN:
		case VAR_CONSTANT:
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

Variable* Variable_copy(const Variable* var) {
	Variable* ret;
	char* name = var->name ? strdup(var->name) : NULL;
	
	switch(var->type) {
		case VAR_BUILTIN:
			ret = VarBuiltin(name, Builtin_copy(var->blt));
			break;
		
		case VAR_CONSTANT:
			ret = VarConstant(name, Builtin_copy(var->blt));
		
		case VAR_VALUE:
			ret = VarValue(name, Value_copy(var->val));
			break;
		
		case VAR_FUNC:
			ret = VarFunc(name, Function_copy(var->func));
			break;
		
		case VAR_ERR:
			ret = VarErr(Error_copy(var->err));
			break;
		
		default:
			badVarType(var->type);
	}
	
	return ret;
}

Value* Variable_eval(const Variable* var, const Context* ctx) {
	Value* ret;
	
	switch(var->type) {
		case VAR_VALUE:
			ret = Value_copy(var->val);
			break;
		
		case VAR_CONSTANT:
			ret = Builtin_eval(var->blt, ctx, NULL, false);
			break;
		
		case VAR_FUNC:
		case VAR_BUILTIN:
			ret = ValVar(var->name);
			break;
		
		case VAR_ERR:
			ret = ValErr(var->err);
			break;
		
		default:
			badVarType(var->type);
	}
	
	return ret;
}

Value* Variable_coerce(const Variable* var, const Context* ctx) {
	Value* ret;
	
	if(var->type == VAR_VALUE) {
		ret = Value_copy(var->val);
	}
	else if(var->type == VAR_FUNC) {
		ret = ValErr(typeError("Variable '%s' is a function.", var->name));
	}
	else if(var->type == VAR_BUILTIN && !var->blt->isFunction) {
		ArgList* noArgs = ArgList_new(0);
		ret = Builtin_eval(var->blt, ctx, noArgs, false);
		ArgList_free(noArgs);
	}
	else {
		badVarType(var->type);
	}
	
	return ret;
}

Variable* Variable_get(const Context* ctx, const char* name) {
	return Context_get(ctx, name);
}

Variable* Variable_getAbove(const Context* ctx, const char* name) {
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

char* Variable_repr(const Variable* var, bool pretty) {
	char* ret;
	
	const char* name = var->name;
	if(pretty) {
		name = getPretty(name);
	}
	
	if(var->type == VAR_FUNC) {
		char* func = Function_repr(var->func, pretty);
		asprintf(&ret, "%s%s", name, func);
		free(func);
	}
	else if(var->type == VAR_BUILTIN) {
		char* blt = Builtin_repr(var->blt, pretty);
		if(name == NULL) {
			ret = blt;
		}
		else {
			asprintf(&ret, "%s = %s", name, blt);
			free(blt);
		}
	}
	else {
		char* val = Value_repr(var->val, pretty);
		if(name == NULL) {
			ret = val;
		}
		else {
			asprintf(&ret, "%s = %s", name, val);
			free(val);
		}
	}
	
	return ret;
}

char* Variable_verbose(const Variable* var) {
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

char* Variable_xml(const Variable* var) {
	/*
	 sc> ?x f(x) = 3x + 4
	 
	 <vardata name="f">
	   <func>
	     <argnames>
		   <arg name="x"/>
	     </argnames>
	     <expr>
	       <add>
	         <mul>
	           <int>3</int>
	           <var name="x"/>
	         </mul>
	         <int>4</int>
		   </add>
	     </expr>
	   </func>
	 </vardata>
	*/
	char* ret;
	char* val;
	
	unsigned indent = var->name == NULL ? 0 : 1;
	
	if(var->type == VAR_FUNC) {
		val = Function_xml(var->func, indent);
	}
	else if(var->type == VAR_BUILTIN) {
		val = Builtin_xml(var->blt, indent);
	}
	else {
		val = Value_xml(var->val, indent);
	}
	
	if(var->name == NULL) {
		return val;
	}
	
	asprintf(&ret,
			 "<vardata name=\"%2$s\">\n" /* name */
				 "%1$s%3$s\n"            /* value */
			 "</vardata>",
			 indentation(1),
			 var->name,
			 val);
	
	free(val);
	return ret;
}

