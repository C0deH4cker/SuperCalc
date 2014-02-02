/*
  fraction.h
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_FRACTION_H_
#define _SC_FRACTION_H_

typedef struct Fraction Fraction;
#include "value.h"

struct Fraction {
	long long n;
	long long d;
};

/* Constructor */
Fraction* Fraction_new(long long numerator, long long denominator);

/* Destructor */
void Fraction_free(Fraction* frac);

/* Copying */
Fraction* Fraction_copy(Fraction* frac);

/* In-place simplification */
void Fraction_simplify(Fraction* frac);
void Fraction_reduce(Value* frac);

/* Arithmetic operations */
Value* Fraction_add(Fraction* a, Value* b);
Value* Fraction_sub(Fraction* a, Value* b);
Value* Fraction_mul(Fraction* a, Value* b);
Value* Fraction_div(Fraction* a, Value* b);
Value* Fraction_mod(Fraction* a, Value* b);
Value* Fraction_pow(Fraction* base, Value* exp);
Value* Fraction_rpow(Fraction* exp, Value* base);

/* Comparison */
Value* Fraction_cmp(Fraction* a, Value* b);

/* Conversion */
double Fraction_asReal(Fraction* frac);

/* Printing */
char* Fraction_repr(Fraction* frac, bool pretty);

#endif
