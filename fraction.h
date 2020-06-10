/*
  fraction.h
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_FRACTION_H
#define SC_FRACTION_H

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
Fraction* Fraction_copy(const Fraction* frac);

/* In-place simplification */
void Fraction_simplify(Fraction* frac);
void Fraction_reduce(Value* frac);

/* Arithmetic operations */
Value* Fraction_add(const Fraction* a, const Value* b);
Value* Fraction_sub(const Fraction* a, const Value* b);
Value* Fraction_mul(const Fraction* a, const Value* b);
Value* Fraction_div(const Fraction* a, const Value* b);
Value* Fraction_mod(const Fraction* a, const Value* b);
Value* Fraction_pow(const Fraction* base, const Value* exp);
Value* Fraction_rpow(const Fraction* exp, const Value* base);

/* Comparison */
Value* Fraction_cmp(const Fraction* a, const Value* b);

/* Conversion */
double Fraction_asReal(const Fraction* frac);

/* Printing */
char* Fraction_repr(const Fraction* frac, bool approx);
char* Fraction_xml(const Fraction* frac);

#endif /* SC_FRACTION_H */
