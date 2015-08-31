/*
  defaults_math.c
  SuperCalc

  Created by C0deH4cker on 11/12/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "defaults.h"
#include <math.h>
#include <stdbool.h>

#include "generic.h"
#include "error.h"
#include "context.h"
#include "builtin.h"
#include "arglist.h"
#include "value.h"
#include "binop.h"
#include "vector.h"
#include "fraction.h"
#include "funccall.h"
#include "template.h"


EVAL_CONST(pi, M_PI);
EVAL_CONST(e, M_E);
#define PHI 1.61803398874989484820458683436563811 /* Golden ratio: (1 + sqrt(5)) / 2 */
EVAL_CONST(phi, PHI);


static Value* eval_sqrt(const Context* ctx, const ArgList* arglist, bool internal) {
	if(arglist->count != 1) {
		return ValErr(builtinArgs("sqrt", 1, arglist->count));
	}
	
	return TP_EVAL("@@^(1/2)", ctx, Value_copy(arglist->args[0]));
}

static Value* eval_abs(const Context* ctx, const ArgList* arglist, bool internal) {
	if(arglist->count != 1) {
		return ValErr(builtinArgs("abs", 1, arglist->count));
	}
	
	Value* val = Value_coerce(arglist->args[0], ctx);
	
	if(val->type == VAL_ERR) {
		return val;
	}
	
	switch(val->type) {
		case VAL_INT:
			return ValInt(ABS(val->ival));
		
		case VAL_REAL:
			return ValReal(ABS(val->rval));
		
		case VAL_FRAC:
			return ValFrac(Fraction_new(ABS(val->frac->n), val->frac->d));
			
		case VAL_VEC:
			if(internal) {
				return Vector_magnitude(val->vec, ctx);
			}
			return ValCall(FuncCall_create("@map", ArgList_create(2, ValVar("abs"), Value_copy(val))));
		
		default:
			badValType(val->type);
	}
	
	Value_free(val);
}

static Value* eval_exp(const Context* ctx, const ArgList* arglist, bool internal) {
	if(arglist->count != 1) {
		return ValErr(builtinArgs("exp", 1, arglist->count));
	}
	
	return ValExpr(BinOp_new(BIN_POW, ValReal(M_E), Value_copy(arglist->args[0])));
}

/* Trigonometric */
EVAL_FUNC(sin, sin(a[0]), 1);
EVAL_FUNC(cos, cos(a[0]), 1);
EVAL_FUNC(tan, tan(a[0]), 1);

EVAL_FUNC(sec, 1 / cos(a[0]), 1);
EVAL_FUNC(csc, 1 / sin(a[0]), 1);
EVAL_FUNC(cot, 1 / tan(a[0]), 1);

EVAL_FUNC(asin, asin(a[0]), 1);
EVAL_FUNC(acos, acos(a[0]), 1);
EVAL_FUNC(atan, atan(a[0]), 1);

EVAL_FUNC(atan2, atan2(a[0], a[1]), 2);

EVAL_FUNC(asec, acos(1 / a[0]), 1);
EVAL_FUNC(acsc, asin(1 / a[0]), 1);
EVAL_FUNC(acot, atan(1 / a[0]), 1);

/* Hyperbolic */
EVAL_FUNC(sinh, sinh(a[0]), 1);
EVAL_FUNC(cosh, cosh(a[0]), 1);
EVAL_FUNC(tanh, tanh(a[0]), 1);

EVAL_FUNC(sech, 1 / cosh(a[0]), 1);
EVAL_FUNC(csch, 1 / sinh(a[0]), 1);
EVAL_FUNC(coth, 1 / tanh(a[0]), 1);

EVAL_FUNC(asinh, asinh(a[0]), 1);
EVAL_FUNC(acosh, acosh(a[0]), 1);
EVAL_FUNC(atanh, atanh(a[0]), 1);

EVAL_FUNC(asech, acosh(1 / a[0]), 1);
EVAL_FUNC(acsch, asinh(1 / a[0]), 1);
EVAL_FUNC(acoth, atanh(1 / a[0]), 1);

EVAL_FUNC(log, log10(a[0]), 1);
EVAL_FUNC(log2, log2(a[0]), 1);
EVAL_FUNC(ln, log(a[0]), 1);
EVAL_FUNC(logbase, log(a[0]) / log(a[1]), 2);


static const char* _math_const_names[] = {
	"pi", "e", "phi"
};

static builtin_eval_t _math_consts[] = {
	&eval_pi, &eval_e, &eval_phi
};

static const char* _math_names[] = {
	"sqrt", "abs", "exp",
	"sin", "cos", "tan",
	"sec", "csc", "cot",
	"asin", "acos", "atan",
	"asec", "acsc", "acot",
	"sinh", "cosh", "tanh",
	"sech", "csch", "coth",
	"asinh", "acosh", "atanh",
	"asech", "acsch", "acoth",
	"log", "log2", "ln",
	"logbase", "atan2"
};

static builtin_eval_t _math_funcs[] = {
	&eval_sqrt, &eval_abs, &eval_exp,
	&eval_sin, &eval_cos, &eval_tan,
	&eval_sec, &eval_csc, &eval_cot,
	&eval_asin, &eval_acos, &eval_atan,
	&eval_asec, &eval_acsc, &eval_acot,
	&eval_sinh, &eval_cosh, &eval_tanh,
	&eval_sech, &eval_csch, &eval_coth,
	&eval_asinh, &eval_acosh, &eval_atanh,
	&eval_asech, &eval_acsch, &eval_acoth,
	&eval_log, &eval_log2, &eval_ln,
	&eval_logbase, &eval_atan2
};


void register_math(Context* ctx) {
	unsigned i;
	unsigned constCount = ARRSIZE(_math_const_names);
	
	for(i = 0; i < constCount; i++) {
		Builtin* blt = Builtin_new(_math_const_names[i], _math_consts[i], false);
		Builtin_register(blt, ctx);
	}
	
	unsigned funcCount = ARRSIZE(_math_names);
	
	for(i = 0; i < funcCount; i++) {
		Builtin* blt = Builtin_new(_math_names[i], _math_funcs[i], true);
		Builtin_register(blt, ctx);
	}
}

