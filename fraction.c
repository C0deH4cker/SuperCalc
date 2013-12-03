/*
 fraction.c
 SuperCalc
 
 Created by C0deH4cker on 10/21/13.
 Copyright (c) 2013 C0deH4cker. All rights reserved.
 */

#include "fraction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "error.h"
#include "generic.h"
#include "value.h"


static Value* fracAdd(Fraction* a, Fraction* b);
static Value* fracSub(Fraction* a, Fraction* b);
static Value* fracMul(Fraction* a, Fraction* b);
static Value* fracDiv(Fraction* a, Fraction* b);
static Value* fracMod(Fraction* a, Fraction* b);
static Value* fracPow(Fraction* base, Fraction* exp);
static int fracCmp(Fraction* a, Fraction* b);


Fraction* Fraction_new(long long numerator, long long denominator) {
	Fraction* ret = fmalloc(sizeof(*ret));
	
	ret->n = numerator * (denominator < 0 ? -1 : 1);
	ret->d = ABS(denominator);
	
	Fraction_simplify(ret);
	
	return ret;
}

void Fraction_free(Fraction* frac) {
	free(frac);
}

Fraction* Fraction_copy(Fraction* frac) {
	return Fraction_new(frac->n, frac->d);
}

void Fraction_simplify(Fraction* frac) {
	long long factor = gcd(ABS(frac->n), frac->d);
	
	frac->n /= factor;
	frac->d /= factor;
}

void Fraction_reduce(Value* frac) {
	Fraction* f = frac->frac;
	Fraction_simplify(f);
	
	if(f->d == 1) {
		memset(frac, 0, sizeof(*frac));
		
		frac->type = VAL_INT;
		frac->ival = f->n;
		
		Fraction_free(f);
	}
}

static Value* fracAdd(Fraction* a, Fraction* b) {
	long long n = a->n * b->d + a->d * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_add(Fraction* a, Value* b) {
	Value* ret;
	long long n, d;
	
	switch(b->type) {
		case VAL_FRAC:
			ret = fracAdd(a, b->frac);
			break;
			
		case VAL_INT:
			n = a->n + b->ival * a->d;
			d = a->d;
			
			ret = ValFrac(Fraction_new(n, d));
			break;
			
		case VAL_REAL:
			ret = ValReal(Fraction_asReal(a) + b->rval);
			break;
			
		default:
			badValType(b->type);
	}
	
	return ret;
}

static Value* fracSub(Fraction* a, Fraction* b) {
	long long n = a->n * b->d - a->d * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_sub(Fraction* a, Value* b) {
	Value* ret;
	long long n, d;
	
	switch(b->type) {
		case VAL_FRAC:
			ret = fracSub(a, b->frac);
			break;
			
		case VAL_INT:
			n = a->n - b->ival * a->d;
			d = a->d;
			
			ret = ValFrac(Fraction_new(n, d));
			break;
			
		case VAL_REAL:
			ret = ValReal(Fraction_asReal(a) - b->rval);
			break;
			
		default:
			badValType(b->type);
			break;
	}
	
	return ret;
}

static Value* fracMul(Fraction* a, Fraction* b) {
	long long n = a->n * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_mul(Fraction* a, Value* b) {
	Value* ret;
	long long n, d;
	
	switch(b->type) {
		case VAL_FRAC:
			ret = fracMul(a, b->frac);
			break;
			
		case VAL_INT:
			n = a->n * b->ival;
			d = a->d;
			
			ret = ValFrac(Fraction_new(n, d));
			break;
			
		case VAL_REAL:
			ret = ValReal(Fraction_asReal(a) * b->rval);
			break;
			
		default:
			badValType(b->type);
			break;
	}
	
	return ret;
}

static Value* fracDiv(Fraction* a, Fraction* b) {
	long long n = a->n * b->d;
	long long d = a->d * b->n;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_div(Fraction* a, Value* b) {
	Value* ret;
	long long n, d;
	
	switch(b->type) {
		case VAL_FRAC:
			ret = fracDiv(a, b->frac);
			break;
			
		case VAL_INT:
			n = a->n;
			d = a->d * b->ival;
			
			ret = ValFrac(Fraction_new(n, d));
			break;
			
		case VAL_REAL:
			ret = ValReal(Fraction_asReal(a) / b->rval);
			break;
			
		default:
			badValType(b->type);
			break;
	}
	
	return ret;
}

static Value* fracMod(Fraction* a, Fraction* b) {
	Value* f;
	Value* next;
	Value* diff;
	
	f = ValFrac(Fraction_copy(b));
	next = Value_copy(f);
	
	/* While the next multiple is still valid, set f to next */
	for(diff = Fraction_cmp(a, next); diff->ival > 0; diff = Fraction_cmp(a, next)) {
		Value_free(f);
		f = Value_copy(next);
		
		/* Calculate next multiple and swap into it */
		Value* tmp = Fraction_add(b, next);
		Value_free(next);
		next = tmp;
	}
	
	Value_free(next);
	
	return Fraction_sub(a, f);
}

Value* Fraction_mod(Fraction* a, Value* b) {
	Value* ret;
	Fraction* f;
	
	if(Fraction_cmp(a, b) < 0) {
		ret = ValFrac(a);
	}
	else {
		/*
		 Left > right at this point. Result of modulus is a - f, where f is the largest
		 multiple of b such that f <= a;
		*/
		
		switch(b->type) {
			case VAL_FRAC:
				ret = fracMod(a, b->frac);
				break;
				
			case VAL_INT:
				f = Fraction_new(b->ival, 1);
				ret = fracMod(a, f);
				Fraction_free(f);
				break;
				
			case VAL_REAL:
				ret = ValReal(remainder(Fraction_asReal(a), b->rval));
				break;
				
			default:
				badValType(b->type);
				break;
		}
	}
	
	return ret;
}

static Value* fracPow(Fraction* base, Fraction* exp) {
	Value* ret;
	long long n, d;
	
	/* c/1 == c */
	if(exp->d == 1) {
		if(exp->n < 0) {
			n = ipow(base->d, -exp->n);
			d = ipow(base->n, -exp->n);
		}
		else {
			n = ipow(base->n, exp->n);
			d = ipow(base->d, exp->n);
		}
		
		ret = ValFrac(Fraction_new(n, d));
	}
	else {
		/*
		 Not a simple integral power. Just treat as a real for now
		 TODO: Perform prime factorization on the base to determine if
		 the power has an integral result.
		*/
		ret = ValReal(pow(Fraction_asReal(base), Fraction_asReal(exp)));
	}
	
	return ret;
}

Value* Fraction_pow(Fraction* base, Value* exp) {
	Value* ret;
	long long n, d;
	
	switch(exp->type) {
		case VAL_FRAC:
			ret = fracPow(base, exp->frac);
			break;
			
		case VAL_INT:
			/* (a/b)^-c is same as (b/a)^c */
			if(exp->ival < 0) {
				n = ipow(base->d, -exp->ival);
				d = ipow(base->n, -exp->ival);
			}
			else {
				n = ipow(base->n, exp->ival);
				d = ipow(base->d, exp->ival);
			}
			
			ret = ValFrac(Fraction_new(n, d));
			break;
			
		case VAL_REAL:
			ret = ValReal(pow(Fraction_asReal(base), exp->rval));
			break;
			
		default:
			badValType(exp->type);
			break;
	}
	
	return ret;
}

static int fracCmp(Fraction* a, Fraction* b) {
	int ret;
	long long val = a->n * b->d - b->n * a->d;
	
	if(val < 0)
		ret = -1;
	else if(val > 0)
		ret = 1;
	else
		ret = 0;
	
	return ret;
}

Value* Fraction_cmp(Fraction* a, Value* b) {
	int diff;
	long long val;
	double real;
	
	switch(b->type) {
		case VAL_INT:
			val = a->n - b->ival * a->d;
			if(val < 0)
				diff = -1;
			else if(val > 0)
				diff = 1;
			else
				diff = 0;
			break;
		
		case VAL_FRAC:
			diff = fracCmp(a, b->frac);
			break;
		
		case VAL_REAL:
			real = Fraction_asReal(a) - b->rval;
			if(real < 0)
				diff = -1;
			else if(real > 0)
				diff = 1;
			else
				diff = 0;
			break;
		
		default:
			badValType(b->type);
	}
	
	return ValInt(diff);
}

double Fraction_asReal(Fraction* frac) {
	return (double)frac->n / (double)frac->d;
}

char* Fraction_repr(Fraction* f, bool pretty) {
	char* ret;
	
	if(pretty)
		asprintf(&ret, "%lld/%lld (%.*g)", f->n, f->d, DBL_DIG, Fraction_asReal(f));
	else
		asprintf(&ret, "%lld/%lld", f->n, f->d);
	
	return ret;
}

