/*
  defaults.h
  SuperCalc

  Created by C0deH4cker on 11/12/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_DEFAULTS_H
#define SC_DEFAULTS_H

#include <stdbool.h>
#include <stdlib.h>

#include "context.h"
#include "generic.h"
#include "value.h"
#include "arglist.h"
#include "error.h"


#define EVAL_CONST(name, val) \
static Value* eval_##name(const Context* ctx, const ArgList* arglist, bool internal) { \
	UNREFERENCED_PARAMETER(ctx); \
	UNREFERENCED_PARAMETER(arglist); \
	UNREFERENCED_PARAMETER(internal); \
	return ValReal((val)); \
}

#define EVAL_FUNC(name, func, nargs) \
static Value* eval_##name(const Context* ctx, const ArgList* arglist, bool internal) { \
	UNREFERENCED_PARAMETER(internal); \
	if(arglist->count != (nargs)) { \
		return ValErr(builtinArgs(#name, (nargs), arglist->count)); \
	} \
	ArgList* e = ArgList_eval(arglist, ctx); \
	if(!e) { \
		return ValErr(ignoreError()); \
	} \
	double* a = ArgList_toReals(e, ctx); \
	if(!a) { \
		return ValErr(badConversion(#name)); \
	} \
	ArgList_free(e); \
	Value* ret = ValReal((func)); \
	free(a); \
	return ret; \
}


void register_math(UNOWNED NONNULL Context* ctx);
void register_vector(UNOWNED NONNULL Context* ctx);


#endif /* SC_DEFAULTS_H */
