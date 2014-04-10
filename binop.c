/*
  binop.c
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "binop.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "error.h"
#include "generic.h"
#include "context.h"
#include "value.h"
#include "fraction.h"
#include "vector.h"

typedef Value* (*binop_t)(Context*, Value*, Value*);

static Value* val_ipow(long long base, long long exp);
static Value* binop_add(Context* ctx, Value* a, Value* b);
static Value* binop_sub(Context* ctx, Value* a, Value* b);
static Value* binop_mul(Context* ctx, Value* a, Value* b);
static Value* binop_div(Context* ctx, Value* a, Value* b);
static Value* binop_mod(Context* ctx, Value* a, Value* b);
static Value* binop_pow(Context* ctx, Value* a, Value* b);
static BINTYPE nextSpecialOp(const char** expr);

static binop_t _binop_table[] = {
	&binop_add,
	&binop_sub,
	&binop_mul,
	&binop_div,
	&binop_mod,
	&binop_pow
};

/* Operator comparison table */
static const int _binop_cmp[6][6] = {
	{0, 0, -1, -1, -1, -1}, /* ADD */
	{0, 0, -1, -1, -1, -1}, /* SUB */
	{1, 1,  0,  0,  0, -1}, /* MUL */
	{1, 1,  0,  0,  0, -1}, /* DIV */
	{1, 1,  0,  0,  0, -1}, /* MOD */
	{1, 1,  1,  1,  0, -1}  /* POW */
};

static const char* _binop_pretty[] = {
	"+", "-", "×", "÷", "%", "^", "$", "?"
};
static const char* _binop_repr[] = {
	"+", "-", "*", "/", "%", "^", "$", "?"
};

const char* binop_verb[] = {
	"add", "subtract", "multiply", "divide", "modulo", "exponentiate"
};


static Value* val_ipow(long long base, long long exp) {
	long long result;
	
	if(exp < 0) {
		/* base^-exp is same as 1/base^exp */
		result = ipow(base, -exp);
		
		return ValFrac(Fraction_new(1, result));
	}
	
	result = ipow(base, exp);
	
	return ValInt(result);
}

static Value* binop_add(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_VEC) {
		/* Let the vector class handle the operation */
		ret = Vector_add(a->vec, b, ctx);
	}
	else if(b->type == VAL_VEC) {
		ret = Vector_add(b->vec, a, ctx);
	}
	else if(a->type == VAL_FRAC) {
		/* Let the fraction class handle the operation */
		ret = Fraction_add(a->frac, b);
	}
	else if(b->type == VAL_FRAC) {
		/* a + (b/c) is same as (b/c) + a */
		ret = Fraction_add(b->frac, a);
	}
	else if(a->type == VAL_INT && b->type == VAL_INT) {
		ret = ValInt(a->ival + b->ival);
	}
	else {
		double a1, a2;
		
		if(a->type == VAL_INT) {
			a1 = a->ival;
		}
		else if(a->type == VAL_REAL) {
			a1 = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_INT) {
			a2 = b->ival;
		}
		else if(b->type == VAL_REAL) {
			a2 = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		ret = ValReal(a1 + a2);
	}
	
	return ret;
}

static Value* binop_sub(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_VEC) {
		ret = Vector_sub(a->vec, b, ctx);
	}
	else if(b->type == VAL_VEC) {
		ret = Vector_rsub(b->vec, a, ctx);
	}
	else if(a->type == VAL_FRAC) {
		ret = Fraction_sub(a->frac, b);
	}
	else if(b->type == VAL_FRAC) {
		/* a - (b/c) is same as (-b/c) + a */
		Fraction* f = Fraction_new(-b->frac->n, b->frac->d);
		
		ret = Fraction_add(f, a);
		
		Fraction_free(f);
	}
	else if(a->type == VAL_INT && b->type == VAL_INT) {
		ret = ValInt(a->ival - b->ival);
	}
	else {
		double s1, s2;
		
		if(a->type == VAL_INT) {
			s1 = a->ival;
		}
		else if(a->type == VAL_REAL) {
			s1 = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_INT) {
			s2 = b->ival;
		}
		else if(b->type == VAL_REAL) {
			s2 = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		ret = ValReal(s1 - s2);
	}
	
	return ret;
}

static Value* binop_mul(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_VEC) {
		ret = Vector_mul(a->vec, b, ctx);
	}
	else if(b->type == VAL_VEC) {
		ret = Vector_mul(b->vec, a, ctx);
	}
	else if(a->type == VAL_FRAC) {
		ret = Fraction_mul(a->frac, b);
	}
	else if(b->type == VAL_FRAC) {
		/* a * (b/c) is same as (b/c) * a */
		ret = Fraction_mul(b->frac, a);
	}
	else if(a->type == VAL_INT && b->type == VAL_INT) {
		ret = ValInt(a->ival * b->ival);
	}
	else {
		double m1, m2;
		
		if(a->type == VAL_INT) {
			m1 = a->ival;
		}
		else if(a->type == VAL_REAL) {
			m1 = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_INT) {
			m2 = b->ival;
		}
		else if(b->type == VAL_REAL) {
			m2 = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		ret = ValReal(m1 * m2);
	}
	
	return ret;
}

static Value* binop_div(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_VEC) {
		ret = Vector_div(a->vec, b, ctx);
	}
	else if(b->type == VAL_VEC) {
		ret = Vector_rdiv(b->vec, a, ctx);
	}
	else if(a->type == VAL_FRAC) {
		ret = Fraction_div(a->frac, b);
	}
	else if(b->type == VAL_FRAC) {
		/* a / (b/c) is same as (c/b) * a */
		Fraction* f = Fraction_new(b->frac->d, b->frac->n);
		
		ret = Fraction_mul(f, a);
		
		Fraction_free(f);
	}
	else if(a->type == VAL_INT && b->type == VAL_INT) {
		if(b->ival == 0) {
			ret = ValErr(zeroDivError());
		}
		else if(a->ival % b->ival == 0) {
			ret = ValInt(a->ival / b->ival);
		}
		else {
			ret = ValFrac(Fraction_new(a->ival, b->ival));
		}
	}
	else {
		double n, d;
		
		if(a->type == VAL_INT) {
			n = a->ival;
		}
		else if(a->type == VAL_REAL) {
			n = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_INT) {
			d = b->ival;
		}
		else if(b->type == VAL_REAL) {
			d = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		if(d == 0) {
			ret = ValErr(zeroDivError());
		}
		else {
			ret = ValReal(n / d);
		}
	}
	
	return ret;
}

static Value* binop_mod(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_VEC || b->type == VAL_VEC) {
		ret = ValErr(typeError("Modulus is not supported for vectors."));
	}
	else if(a->type == VAL_INT && b->type == VAL_INT) {
		if(b->ival == 0) {
			ret = ValErr(zeroModError());
		}
		else {
			ret = ValInt(a->ival % b->ival);
		}
	}
	else if(a->type == VAL_FRAC) {
		ret = Fraction_mod(a->frac, b);
	}
	else if(a->type == VAL_INT && b->type == VAL_FRAC) {
		/* a % (b/c) is same as (a/1) % (b/c) */
		Fraction* f = Fraction_new(a->ival, 1);
		
		ret = Fraction_mod(f, b);
		
		Fraction_free(f);
	}
	else {
		double n, d;
		
		if(a->type == VAL_INT) {
			n = a->ival;
		}
		else if(a->type == VAL_REAL) {
			n = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_INT) {
			d = b->ival;
		}
		else if(b->type == VAL_REAL) {
			d = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		if(d == 0) {
			ret = ValErr(zeroDivError());
		}
		else {
			ret = ValReal(remainder(n, d));
		}
	}
	
	return ret;
}

static Value* binop_pow(Context* ctx, Value* a, Value* b) {
	Value* ret;
	
	if(a->type == VAL_INT && b->type == VAL_INT) {
		ret = val_ipow(a->ival, b->ival);
	}
	else if(a->type == VAL_VEC) {
		ret = Vector_pow(a->vec, b, ctx);
	}
	else if(b->type == VAL_VEC) {
		ret = Vector_rpow(b->vec, a, ctx);
	}
	else if(a->type == VAL_FRAC) {
		ret = Fraction_pow(a->frac, b);
	}
	else if(b->type == VAL_FRAC) {
		ret = Fraction_rpow(b->frac, a);
	}
	else {
		/* Just do a real pow */
		double base, exp;
		
		if(a->type == VAL_INT) {
			base = a->ival;
		}
		else if(a->type == VAL_REAL) {
			base = a->rval;
		}
		else {
			return ValErr(badOpType("left", a->type));
		}
		
		if(b->type == VAL_FRAC) {
			exp = Fraction_asReal(b->frac);
		}
		else if(b->type == VAL_INT) {
			exp = b->ival;
		}
		else if(b->type == VAL_REAL) {
			exp = b->rval;
		}
		else {
			return ValErr(badOpType("rightf", b->type));
		}
		
		ret = ValReal(pow(base, exp));
	}
	
	return ret;
}

BinOp* BinOp_new(BINTYPE type, Value* a, Value* b) {
	BinOp* ret = fmalloc(sizeof(*ret));
	
	memset(ret, 0, sizeof(*ret));
	
	ret->type = type;
	ret->a = a;
	ret->b = b;
	
	return ret;
}

void BinOp_free(BinOp* node) {
	if(node == NULL) return;
	
	/* Free child values */
	Value_free(node->a);
	Value_free(node->b);
	
	/* Free self */
	free(node);
}

BinOp* BinOp_copy(BinOp* node) {
	return BinOp_new(node->type, Value_copy(node->a), Value_copy(node->b));
}

Value* BinOp_eval(BinOp* node, Context* ctx) {
	if(node == NULL) {
		return ValErr(nullError());
	}
	
	Value* a = Value_coerce(node->a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* b = Value_coerce(node->b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
	Value* ret = _binop_table[node->type](ctx, a, b);
	
	Value_free(a);
	Value_free(b);
	
	return ret;
}

/* Like Rambo */
static BINTYPE nextSpecialOp(const char** expr) {
	unsigned i;
	for(i = 0; i < ARRSIZE(_binop_pretty); i++) {
		size_t len = strlen(_binop_pretty[i]);
		
		if(strncmp(_binop_pretty[i], *expr, len) == 0) {
			*expr += len;
			return i;
		}
	}
	
	return BIN_UNK;
}

BINTYPE BinOp_nextType(const char** expr, char sep, char end) {
	BINTYPE ret;
	
	trimSpaces(expr);
	
	if(**expr == sep || **expr == end) {
		return BIN_END;
	}
	
	switch(**expr) {
		case '+':
			ret = BIN_ADD;
			break;
		case '-':
			ret = BIN_SUB;
			break;
		case '*':
			ret = BIN_MUL;
			break;
		case '/':
			ret = BIN_DIV;
			break;
		case '%':
			ret = BIN_MOD;
			break;
		case '^':
			ret = BIN_POW;
			break;
		
		case '\0':
			return BIN_END;
		
		default:
			/* This handles cases like 2sqrt(5) -> 2 * sqrt(5) */
			if(**expr == '(' || **expr == '<' || isalpha(**expr)) {
				return BIN_MUL;
			}
			
			return nextSpecialOp(expr);
	}
	
	(*expr)++;
	return ret;
}

/*
 Determine which operator has a higher precedence.
 -1 means a has a lower precedence than b, 1 means
 a has a higher precedence than b, and 0 means that
 both a and b have equal precedence.
*/
int BinOp_cmp(BINTYPE a, BINTYPE b) {
	return _binop_cmp[a][b];
}

char* BinOp_verbose(BinOp* op, int indent) {
	char* ret;
	
	if(op == NULL) {
		return strNULL();
	}
	
	char* spacing = spaces(indent + IWIDTH);
	char* current = spaces(indent);
	
	char* a = Value_verbose(op->a, indent + IWIDTH);
	char* b = Value_verbose(op->b, indent + IWIDTH);
	
	asprintf(&ret, "%s (\n%s[a] %s\n%s[b] %s\n%s)",
			 _binop_repr[op->type],
			 spacing, a,
			 spacing, b,
			 current);
	
	free(spacing);
	free(current);
	free(a);
	free(b);
	
	return ret;
}

char* BinOp_repr(BinOp* node, bool pretty) {
	char* ret;
	
	if(node == NULL) {
		return strNULL();
	}
	
	char* a = Value_repr(node->a, pretty);
	char* b = Value_repr(node->b, pretty);
	
	const char* opstr = (pretty ? _binop_pretty : _binop_repr)[node->type];
	
	asprintf(&ret, "%s %s %s", a, opstr, b);
	
	free(a);
	free(b);
	
	return ret;
}

