/*
  defaults.h
  SuperCalc

  Created by C0deH4cker on 11/12/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_DEFAULTS_H_
#define _SC_DEFAULTS_H_

#include "context.h"


#define EVAL_CONST(name, val) static Value* eval_ ## name(Context* ctx, ArgList* arglist, bool internal) { \
	return ValReal((val)); \
}

#define EVAL_FUNC(name, func, nargs) static Value* eval_ ## name(Context* ctx, ArgList* arglist, bool internal) { \
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


void register_math(Context* ctx);
void register_vector(Context* ctx);


#endif
