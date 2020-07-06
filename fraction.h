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
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct Fraction {
	/* If the fraction's value is negative, the sign will be on the numerator */
	long long n;
	INVARIANT(d > 0) long long d;
};


DECL(Fraction);

/* Constructor */
RETURNS_OWNED Fraction* Fraction_new(long long numerator, INVARIANT(denominator != 0) long long denominator);

/* Destructor */
void Fraction_free(CONSUMED Fraction* _Nullable frac);

/* Copying */
RETURNS_OWNED Fraction* Fraction_copy(const Fraction* frac);

/* In-place simplification */
void Fraction_simplify(INOUT Fraction* frac);
void Fraction_reduce(INOUT Value* frac);

/* Arithmetic operations */
RETURNS_OWNED Value* Fraction_add(const Fraction* a, const Value* b);
RETURNS_OWNED Value* Fraction_sub(const Fraction* a, const Value* b);
RETURNS_OWNED Value* Fraction_mul(const Fraction* a, const Value* b);
RETURNS_OWNED Value* Fraction_div(const Fraction* a, const Value* b);
RETURNS_OWNED Value* Fraction_mod(const Fraction* a, const Value* b);
RETURNS_OWNED Value* Fraction_pow(const Fraction* base, const Value* exp);
RETURNS_OWNED Value* Fraction_rpow(const Fraction* exp, const Value* base);

/* Comparison */
int Fraction_cmp(const Fraction* a, const Value* b);

/* Conversion */
double Fraction_asReal(const Fraction* frac);

/* Printing */
RETURNS_OWNED char* Fraction_repr(const Fraction* frac, bool approx);
RETURNS_OWNED char* Fraction_xml(const Fraction* frac);

METHOD_debugString(Fraction);

ASSUME_NONNULL_END

#endif /* SC_FRACTION_H */
