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

#include "support.h"
#include "error.h"
#include "generic.h"
#include "context.h"
#include "value.h"
#include "fraction.h"
#include "vector.h"

typedef Value* (*binop_t)(const Context*, const Value*, const Value*);

static Value* val_ipow(long long base, long long exp);
static Value* binop_add(const Context* ctx, const Value* a, const Value* b);
static Value* binop_sub(const Context* ctx, const Value* a, const Value* b);
static Value* binop_mul(const Context* ctx, const Value* a, const Value* b);
static Value* binop_div(const Context* ctx, const Value* a, const Value* b);
static Value* binop_mod(const Context* ctx, const Value* a, const Value* b);
static Value* binop_pow(const Context* ctx, const Value* a, const Value* b);
static BINTYPE nextSpecialOp(const char** expr);

static binop_t _binop_table[BIN_COUNT] = {
	&binop_add,
	&binop_sub,
	&binop_mul,
	&binop_div,
	&binop_mod,
	&binop_pow
};

/* Operator comparison table */
static const int _binop_cmp[BIN_COUNT+1][BIN_COUNT+1] = {
	             /* ADD  SUB  MUL  DIV  MOD  POW  HIGHEST */
	/* ADD */     {   0,   0,  -1,  -1,  -1,  -1,  -1},
	/* SUB */     {   0,   0,  -1,  -1,  -1,  -1,  -1},
	/* MUL */     {   1,   1,   0,   0,   0,  -1,  -1},
	/* DIV */     {   1,   1,   0,   0,   0,  -1,  -1},
	/* MOD */     {   1,   1,   0,   0,   0,  -1,  -1},
	/* POW */     {   1,   1,   1,   1,   0,  -1,  -1},
	/* HIGHEST */ {   1,   1,   1,   1,   1,   1,   1}
};

static const char* _binop_pretty[BIN_COUNT] = {
	"+", "-", "×", "÷", "%", "^"
};
static const char* _binop_repr[BIN_COUNT] = {
	"+", "-", "*", "/", "%", "^"
};
static const char* _binop_xml[BIN_COUNT] = {
	"add", "sub", "mul", "div", "mod", "pow"
};

const char* binop_verb[BIN_COUNT] = {
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

static Value* binop_add(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
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

static Value* binop_sub(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
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

static Value* binop_mul(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	/* Short circuit evaluation of multiplication when the left side is zero */
	double a_real = Value_asReal(a);
	if(!isnan(a_real) && !a_real) {
		return ValInt(0);
	}
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
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

static Value* binop_div(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
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

static Value* binop_mod(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		Value_free(a);
		return b;
	}
	
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
			ret = ValReal(fmod(n, d));
		}
	}
	
	return ret;
}

static Value* binop_pow(const Context* ctx, const Value* val_a, const Value* val_b) {
	Value* ret;
	
	Value* b = Value_eval(val_b, ctx);
	if(b->type == VAL_ERR) {
		return b;
	}
	
	/* Shortcut execution of x^0 to not evaluate x */
	double b_real = Value_asReal(b);
	if(!isnan(b_real) && !b_real) {
		return ValInt(1);
	}
	
	Value* a = Value_eval(val_a, ctx);
	if(a->type == VAL_ERR) {
		Value_free(b);
		return a;
	}
	
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
		
		if(b->type == VAL_INT) {
			exp = b->ival;
		}
		else if(b->type == VAL_REAL) {
			exp = b->rval;
		}
		else {
			return ValErr(badOpType("right", b->type));
		}
		
		ret = ValReal(pow(base, exp));
	}
	
	return ret;
}

BinOp* BinOp_new(BINTYPE type, Value* a, Value* b) {
	BinOp* ret = fmalloc(sizeof(*ret));
	
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

BinOp* BinOp_copy(const BinOp* node) {
	if(node == NULL) {
		return NULL;
	}
	
	return BinOp_new(node->type, Value_copy(node->a), Value_copy(node->b));
}

void BinOp_setScope(BinOp* node, const Context* ctx) {
	Value_setScope(node->a, ctx);
	Value_setScope(node->b, ctx);
}

Value* BinOp_eval(const BinOp* node, const Context* ctx) {
	if(node == NULL) {
		return ValErr(nullError());
	}
	
	return _binop_table[node->type](ctx, node->a, node->b);
}

/* Like Rambo */
static BINTYPE nextSpecialOp(const char** expr) {
	BINTYPE i;
	for(i = BIN_ADD; i < BIN_AFTERMAX; i++) {
		size_t len = strlen(_binop_pretty[i]);
		
		if(strncmp(_binop_pretty[i], *expr, len) == 0) {
			*expr += len;
			return i;
		}
	}
	
	return BIN_UNK;
}

BINTYPE BinOp_nextType(const char** expr, char sep, char end) {
	BINTYPE ret = BIN_UNK;
	
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
			if(**expr == '(' || **expr == '<' || isalpha(**expr) || **expr == '_') {
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

char* BinOp_repr(const BinOp* node, bool pretty) {
	char* ret;
	const char* opstr = (pretty ? _binop_pretty : _binop_repr)[node->type];
	
	char* strs[2] = {Value_repr(node->a, pretty, false), Value_repr(node->b, pretty, false)};
	BINTYPE types[2] = {BIN_AFTERMAX, BIN_AFTERMAX};
	
	/* Determine expr type of a */
	if(node->a->type == VAL_FRAC) {
		types[0] = BIN_DIV;
	}
	else if(node->a->type == VAL_EXPR) {
		types[0] = node->a->expr->type;
	}
	
	/* Determine expr type of b */
	if(node->b->type == VAL_FRAC) {
		types[1] = BIN_DIV;
	}
	else if(node->b->type == VAL_EXPR) {
		types[1] = node->b->expr->type;
	}
	
	/* Determine whether the subexpression needs to be parenthesized */
	unsigned i;
	for(i = 0; i < 2; i++) {
		if(BinOp_cmp(node->type, types[i]) > 0) {
			char* tmp;
			asprintf(&tmp, "(%s)", strs[i]);
			free(strs[i]);
			strs[i] = tmp;
		}
	}
	
	asprintf(&ret, "%s %s %s", strs[0], opstr, strs[1]);
	
	free(strs[0]);
	free(strs[1]);
	return ret;
}

char* BinOp_wrap(const BinOp* node) {
	char* ret;
	Value* nodes[2] = {node->a, node->b};
	char* strs[2] = {Value_wrap(node->a, false), Value_wrap(node->b, false)};
	
	/* Always parenthesize subexpressions */
	unsigned i;
	for(i = 0; i < 2; i++) {
		if(nodes[i]->type == VAL_EXPR) {
			char* tmp;
			asprintf(&tmp, "(%s)", strs[i]);
			free(strs[i]);
			strs[i] = tmp;
		}
	}
	
	asprintf(&ret, "%s %s %s", strs[0], _binop_repr[node->type], strs[1]);
	
	free(strs[0]);
	free(strs[1]);
	return ret;
}

char* BinOp_verbose(const BinOp* node, unsigned indent) {
	char* ret;
	char* a = Value_verbose(node->a, indent + 1);
	char* b = Value_verbose(node->b, indent + 1);
	
	asprintf(&ret,
			 "%3$s (\n"           /* BinOp type */
				 "%2$s[a] %4$s\n" /* a */
				 "%2$s[b] %5$s\n" /* b */
			 "%1$s)",
			 indentation(indent), indentation(indent + 1),
			 _binop_repr[node->type],
			 a,
			 b);
	
	free(b);
	free(a);
	return ret;
}

char* BinOp_xml(const BinOp* node, unsigned indent) {
	/*
	 sc> ?x 4 + 1 - 3 * 7
	 
	 <sub>
	   <add>
	     <int>4</int>
	     <int>1</int>
	   </add>
	   <mul>
	     <int>3</int>
	     <int>7</int>
	   </mul>
	 </sub>
	 
	 -16
	*/
	char* ret;
	char* a = Value_xml(node->a, indent + 1);
	char* b = Value_xml(node->b, indent + 1);
	
	asprintf(&ret,
			 "<%3$s>\n"
				 "%2$s%4$s\n" /* a */
				 "%2$s%5$s\n" /* b */
			 "%1$s</%3$s>",
			 indentation(indent), indentation(indent + 1),
			 _binop_xml[node->type],
			 a,
			 b);
	
	free(b);
	free(a);
	return ret;
}

