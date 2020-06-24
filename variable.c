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

#include "support.h"
#include "generic.h"
#include "error.h"
#include "value.h"
#include "context.h"
#include "function.h"
#include "builtin.h"
#include "variable.h"
#include "arglist.h"


Variable* Variable_new(char* name, Value* val) {
	Variable* ret = fmalloc(sizeof(*ret));
	ret->name = name;
	ret->val = val;
	return ret;
}

Variable* VarErr(Error* err) {
	return Variable_new(NULL, ValErr(err));
}

void Variable_free(Variable* var) {
	if(!var) {
		return;
	}
	
	destroy(var->name);
	Value_free(var->val);
	destroy(var);
}

Variable* Variable_copy(const Variable* var) {
	char* name = var->name ? strdup(var->name) : NULL;
	return Variable_new(name, Value_copy(var->val));
}

Value* Variable_eval(const Variable* var, const Context* ctx) {
	return Value_eval(var->val, ctx);
}

Variable* Variable_get(const Context* ctx, const char* name) {
	return Context_get(ctx, name);
}

Variable* Variable_getAbove(const Context* ctx, const char* name) {
	return Context_getAbove(ctx, name);
}

void Variable_update(Variable* dst, Value* src) {
	/* Free old value */
	Value_free(dst->val);
	
	/* Move value from src to dst */
	dst->val = src;
}

char* Variable_repr(const Variable* var, bool pretty) {
	char* ret = NULL;
	const char* name = var->name;
	
	/* When the variable contains a function, print it like "f(x) = x + 4" instead of "f = |x| x + 4" */
	if(var->val->type == VAL_FUNC) {
		return Function_repr(var->val->func, name, pretty);
	}
	
	if(pretty) {
		name = getPretty(name);
	}
	
	char* val = Value_repr(var->val, pretty, false);
	
	if(name == NULL) {
		ret = val;
	}
	else {
		asprintf(&ret, "%s = %s", name, val);
		destroy(val);
	}
	
	return ret;
}

char* Variable_wrap(const Variable* var) {
	char* ret = NULL;
	const char* name = var->name;
	
	/* When the variable contains a function, print it like "f(x) = x + 4" instead of "f = |x| x + 4" */
	if(var->val->type == VAL_FUNC) {
		return Function_wrap(var->val->func, name, true);
	}
	
	char* val = Value_wrap(var->val, true);
	
	if(name == NULL) {
		ret = val;
	}
	else {
		asprintf(&ret, "%s = %s", name, val);
		destroy(val);
	}
	
	return ret;
}

char* Variable_verbose(const Variable* var) {
	char* ret = NULL;
	const char* name = var->name;
	
	char* val = Value_verbose(var->val, 0);
	if(name == NULL) {
		ret = val;
	}
	else {
		asprintf(&ret, "%s = %s", name, val);
		destroy(val);
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
	char* ret = NULL;
	
	unsigned indent = var->name == NULL ? 0 : 1;
	
	char* val = Value_xml(var->val, indent);
	
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
	
	destroy(val);
	return ret;
}

