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


typedef struct prime_list {
	long long prime;
	long long count;
} prime_list;

static Value* fracAdd(const Fraction* a, const Fraction* b);
static Value* fracSub(const Fraction* a, const Fraction* b);
static Value* fracMul(const Fraction* a, const Fraction* b);
static Value* fracDiv(const Fraction* a, const Fraction* b);
static Value* fracMod(const Fraction* a, const Fraction* b);
static prime_list* factor_primes(long long n, unsigned* count);
static Value* fracPow(const Fraction* base, const Fraction* exp);
static int fracCmp(const Fraction* a, const Fraction* b);


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

Fraction* Fraction_copy(const Fraction* frac) {
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

static Value* fracAdd(const Fraction* a, const Fraction* b) {
	long long n = a->n * b->d + a->d * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_add(const Fraction* a, const Value* b) {
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

static Value* fracSub(const Fraction* a, const Fraction* b) {
	long long n = a->n * b->d - a->d * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_sub(const Fraction* a, const Value* b) {
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

static Value* fracMul(const Fraction* a, const Fraction* b) {
	long long n = a->n * b->n;
	long long d = a->d * b->d;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_mul(const Fraction* a, const Value* b) {
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

static Value* fracDiv(const Fraction* a, const Fraction* b) {
	long long n = a->n * b->d;
	long long d = a->d * b->n;
	
	return ValFrac(Fraction_new(n, d));
}

Value* Fraction_div(const Fraction* a, const Value* b) {
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

static Value* fracMod(const Fraction* a, const Fraction* b) {
	Value* f;
	Value* next;
	Value* diff;
	
	f = ValFrac(Fraction_copy(b));
	next = Value_copy(f);
	
	/* While the next multiple is still valid, set f to next */
	/* TODO: Fix hang with "2/3%-2" */
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

Value* Fraction_mod(const Fraction* a, const Value* b) {
	Value* ret;
	Fraction* f;
	
	if(Fraction_cmp(a, b) < 0) {
		ret = ValFrac(Fraction_copy(a));
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
				ret = ValReal(fmod(Fraction_asReal(a), b->rval));
				break;
				
			default:
				badValType(b->type);
				break;
		}
	}
	
	return ret;
}

static prime_list* factor_primes(long long n, unsigned* count) {
	unsigned size = 2, len = 0;
	
	if(n <= 1) {
		*count = 0;
		return NULL;
	}
	
	prime_list* ret = fmalloc(size * sizeof(*ret));
	
	/* Handle 2 separately so the loop can be more efficient */
	if((n & 1) == 0) {
		ret[len].prime = 2;
		ret[len].count = 0;
		
		while((n & 1) == 0) {
			n >>= 1;
			ret[len].count++;
		}
		
		len++;
	}
	
	for(int i = 3; n > 1; i += 2) {
		if(n % i == 0) {
			if(len >= size) {
				size *= 2;
				ret = frealloc(ret, size * sizeof(*ret));
			}
			
			ret[len].prime = i;
			ret[len].count = 0;
			
			while(n % i == 0) {
				n /= i;
				ret[len].count++;
			}
			
			len++;
		}
	}
	
	*count = len;
	return ret;
}

static Value* fracPow(const Fraction* base, const Fraction* exp) {
	Value* ret;
	long long n, d;
	
	if(base->n == 0) {
		return ValInt(0);
	}
	
	if(base->n < 0) {
		/* Negative base with fractional exponent results in complex result */
		return ValErr(mathError("Power result is complex"));
	}
	
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
		 (a/b) ^ (c/d) == (a^(c/d)) / (b^(c/d))
		*/
		unsigned i, n_count, d_count;
		prime_list* n_primes = factor_primes(base->n, &n_count);
		prime_list* d_primes = factor_primes(base->d, &d_count);
		
		/* Initialize coefficient fraction to 1 */
		n = d = 1;
		
		/* Simplify the base's numerator */
		for(i = 0; i < n_count; i++) {
			/* Apply the exponent's numerator to each prime */
			n_primes[i].count *= exp->n;
			
			/* TODO: Fix hang with "-(3+4!/7)^-(3+43+4!/7)^-(3+433" */
			while(n_primes[i].count >= exp->d) {
				n *= n_primes[i].prime;
				n_primes[i].count -= exp->d;
			}
		}
		
		/* Now simplify the base's denominator */
		for(i = 0; i < d_count; i++) {
			/* Apply the exponent's numerator to each prime */
			d_primes[i].count *= exp->n;
			
			while(d_primes[i].count >= exp->d) {
				d *= d_primes[i].prime;
				d_primes[i].count -= exp->d;
			}
		}
		
		/*
		 All of the possible simplifications have been done.
		 Now just recombine the prime lists together to form
		 the new and fully reduced base -- unless it was
		 reduced completely.
		*/
		
		Value* coef = ValFrac(Fraction_new(n, d));
		
		/* Completely reduced? */
		bool complete = true;
		long long base_n = 1, base_d = 1;
		for(i = 0; i < n_count; i++) {
			if(n_primes[i].count > 0) {
				complete = false;
				base_n *= ipow(n_primes[i].prime, n_primes[i].count);
			}
		}
		for(i = 0; i < d_count; i++) {
			if(d_primes[i].count > 0) {
				complete = false;
				base_d *= ipow(d_primes[i].prime, d_primes[i].count);
			}
		}
		
		if(complete) {
			/* Completely reduced, so no MUL or POW */
			ret = coef;
		}
		else {
			/* Not completely reduced */
#if 0
			/* TODO: Fix this */
			/* Did any reduction even occur? */
			if(coef->type == VAL_INT && coef->ival == 1) {
				/* No reduction occurred */
				ret = ValExpr(BinOp_new(BIN_POW,
										ValFrac(Fraction_copy(base)),
										ValFrac(Fraction_copy(exp))
										));
			}
			else {
				/* 
				 Partially reduced
				 coef * base ^ exp == MUL(coef, POW(base, exp))
				*/
				ret = ValExpr(BinOp_new(BIN_MUL,
										coef,
										ValExpr(BinOp_new(BIN_POW,
														  ValFrac(Fraction_new(base_n, base_d)),
														  ValFrac(Fraction_copy(exp))
														  ))
										));
			}
#else
			ret = ValReal(pow(Fraction_asReal(base), Fraction_asReal(exp)));
#endif
		}
		
		free(n_primes);
		free(d_primes);
	}
	
	return ret;
}

Value* Fraction_pow(const Fraction* base, const Value* exp) {
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

Value* Fraction_rpow(const Fraction* exp, const Value* base) {
	Value* ret;
	Fraction* fbase;
	
	switch(base->type) {
		case VAL_FRAC:
			/* Shouldn't happen, but easy to add */
			ret = fracPow(base->frac, exp);
			break;
			
		case VAL_INT:
			/* a^(b/c) */
			fbase = Fraction_new(base->ival, 1);
			ret = fracPow(fbase, exp);
			Fraction_free(fbase);
			break;
			
		case VAL_REAL:
			ret = ValReal(pow(base->rval, Fraction_asReal(exp)));
			break;
			
		default:
			badValType(base->type);
			break;
	}
	
	return ret;
}

static int fracCmp(const Fraction* a, const Fraction* b) {
	long long val = a->n * b->d - b->n * a->d;
	
	return (int)CLAMP(val, -1, 1);
}

Value* Fraction_cmp(const Fraction* a, const Value* b) {
	int diff;
	long long val;
	double real;
	
	switch(b->type) {
		case VAL_INT:
			val = a->n - b->ival * a->d;
			diff = (int)CLAMP(val, -1, 1);
			break;
		
		case VAL_FRAC:
			diff = fracCmp(a, b->frac);
			break;
		
		case VAL_REAL:
			real = Fraction_asReal(a) - b->rval;
			diff = (int)CLAMP(real, -1, 1);
			break;
		
		default:
			badValType(b->type);
	}
	
	return ValInt(diff);
}

double Fraction_asReal(const Fraction* frac) {
	return (double)frac->n / (double)frac->d;
}

char* Fraction_repr(const Fraction* f, bool pretty) {
	char* ret;
	
	if(pretty) {
		asprintf(&ret, "%lld/%lld (%.*g)", f->n, f->d, DBL_DIG, Fraction_asReal(f));
	}
	else {
		asprintf(&ret, "%lld/%lld", f->n, f->d);
	}
	
	return ret;
}

