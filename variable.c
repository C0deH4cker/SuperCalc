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


Variable* Variable_new(char* name) {
	Variable* ret = fmalloc(sizeof(*ret));
	ret->name = name;
	ret->val = NULL;
	ret->scope = NULL;
	return ret;
}

void Variable_free(Variable* var) {
	if(!var) {
		return;
	}
	
	free(var->name);
	free(var);
}

Variable* Variable_copy(const Variable* var) {
	if(!var) {
		return NULL;
	}
	
	char* name = var->name ? strdup(var->name) : NULL;
	Variable* ret = Variable_new(name);
	ret->val = var->val;
	ret->scope = var->scope;
	return ret;
}

void Variable_setScope(Variable* var, const Context* scope) {
	var->scope = scope;
}

Value* Variable_lookup(Variable* var, const Context* ctx) {
	if(var->val == NULL) {
		const char* name = var->name;
		if(name[0] == '@') {
			++name;
		}
		
		var->val = Context_get(var->scope ?: ctx, name);
	}
	
	if(var->val == NULL) {
		return ValErr(varNotFound(var->name));
	}
	
	return var->val;
}

void Variable_update(Variable* dst, Value* src) {
	/* Free old value */
	Value_free(dst->val);
	
	/* Move value from src to dst */
	dst->val = src;
}

char* Variable_repr(const char* name, const Value* val, bool pretty) {
	char* ret = NULL;
	
	/* When the variable contains a function, print it like "f(x) = x + 4" instead of "f = |x| x + 4" */
	if(val->type == VAL_FUNC) {
		return Function_repr(val->func, name, pretty);
	}
	
	if(pretty) {
		name = getPretty(name);
	}
	
	char* valstr = Value_repr(val, pretty, false);
	
	if(name == NULL) {
		ret = valstr;
	}
	else {
		asprintf(&ret, "%s = %s", name, valstr);
		free(valstr);
	}
	
	return ret;
}

char* Variable_wrap(const char* name, const Value* val) {
	char* ret = NULL;
	
	/* When the variable contains a function, print it like "f(x) = x + 4" instead of "f = |x| x + 4" */
	if(val->type == VAL_FUNC) {
		return Function_wrap(val->func, name, true);
	}
	
	char* valstr = Value_wrap(val, true);
	
	if(name == NULL) {
		ret = valstr;
	}
	else {
		asprintf(&ret, "%s = %s", name, valstr);
		free(valstr);
	}
	
	return ret;
}

char* Variable_verbose(const char* name, const Value* val) {
	char* ret = NULL;
	
	char* valstr = Value_verbose(val, 0);
	if(name == NULL) {
		ret = valstr;
	}
	else {
		asprintf(&ret, "%s = %s", name, valstr);
		free(valstr);
	}
	
	return ret;
}

char* Variable_xml(const char* name, const Value* val) {
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
	
	unsigned indent = name == NULL ? 0 : 1;
	
	char* valstr = Value_xml(val, indent);
	
	if(name == NULL) {
		return valstr;
	}
	
	asprintf(&ret,
			 "<vardata name=\"%2$s\">\n" /* name */
			     "%1$s%3$s\n"            /* value */
			 "</vardata>",
			 indentation(1),
			 name,
			 valstr);
	
	free(valstr);
	return ret;
}

