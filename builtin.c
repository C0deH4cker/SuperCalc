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
#include <math.h>

#include "error.h"
#include "generic.h"
#include "value.h"
#include "context.h"
#include "arglist.h"


Builtin* Builtin_new(const char* name, builtin_eval_t evaluator) {
	Builtin* ret = fmalloc(sizeof(*ret));
	
	ret->name = strdup(name);
	ret->evaluator = evaluator;
	
	return ret;
}

void Builtin_free(Builtin* blt) {
	free(blt->name);
	free(blt);
}

Builtin* Builtin_copy(Builtin* blt) {
	return Builtin_new(blt->name, blt->evaluator);
}

void Builtin_register(Builtin* blt, Context* ctx) {
	Variable* var = VarBuiltin(blt->name, blt);
	Context_addGlobal(ctx, var);
}

Value* Builtin_eval(Builtin* blt, Context* ctx, ArgList* arglist) {
	/* Call the builtin's evaluator function */
	Value* tmp = blt->evaluator(ctx, arglist);
	
	/* Simplify result */
	Value* ret = Value_eval(tmp, ctx);
	Value_free(tmp);
	
	if(ret->type == VAL_REAL && isnan(ret->rval)) {
		Value_free(ret);
		ret = ValErr(mathError("Builtin function '%s' returned an invalid value.", blt->name));
	}
	
	return ret;
}

char* Builtin_verbose(Builtin* blt, int indent) {
	char* ret;
	
	asprintf(&ret, "<builtin %s>", blt->name);
	
	return ret;
}

char* Builtin_repr(Builtin* blt) {
	return strdup(blt->name);
}

