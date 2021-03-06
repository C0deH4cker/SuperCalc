/*
  builtin.c
  SuperCalc

  Created by C0deH4cker on 11/8/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "support.h"
#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "arglist.h"
#include "variable.h"


Builtin* Builtin_new(char* name, builtin_eval_t evaluator, bool isFunction) {
	Builtin* ret = fmalloc(sizeof(*ret));
	
	ret->name = name;
	ret->evaluator = evaluator;
	ret->isFunction = isFunction;
	
	return ret;
}

void Builtin_free(Builtin* blt) {
	if(!blt) {
		return;
	}
	
	destroy(blt->name);
	destroy(blt);
}

Builtin* Builtin_copy(const Builtin* blt) {
	return Builtin_new(strdup(blt->name), blt->evaluator, blt->isFunction);
}

void Builtin_register(Builtin* blt, Context* ctx) {
	Variable* var = Variable_new(strdup(blt->name), ValBuiltin(blt));
	Context_addGlobal(ctx, var);
}

Value* Builtin_eval(const Builtin* blt, const Context* ctx, const ArgList* arglist, bool internal) {
	/* Call the builtin's evaluator function */
	Value* ret = blt->evaluator(ctx, arglist, internal);
	if(ret->type == VAL_REAL && isnan(ret->rval)) {
		Value_free(ret);
		return ValErr(mathError("Builtin function '%s' returned an invalid value.", blt->name));
	}
	
	return ret;
}

char* Builtin_repr(const Builtin* blt, bool pretty) {
	if(pretty) {
		return strdup(getPretty(blt->name));
	}
	
	return strdup(blt->name);
}

char* Builtin_verbose(const Builtin* blt, unsigned indent) {
	UNREFERENCED_PARAMETER(indent);
	
	char* ret;
	asprintf(&ret, "<builtin %s>", blt->name);
	return ret;
}

char* Builtin_xml(const Builtin* blt, unsigned indent) {
	UNREFERENCED_PARAMETER(indent);
	
	/*
	 sc> ?x sqrt
	 
	 <vardata name="sqrt">
	   <builtin name="sqrt"/>
	 </vardata>
	*/
	char* ret;
	asprintf(&ret, "<builtin name=\"%s\"/>", blt->name);
	return ret;
}

