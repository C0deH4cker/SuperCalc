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

struct Fraction {
	/* If the fraction's value is negative, the sign will be on the numerator */
	long long n;
	INVARIANT(d > 0) long long d;
};

/* Constructor */
OWNED NONNULL Fraction* Fraction_new(long long numerator, INVARIANT(denominator != 0) long long denominator);

/* Destructor */
void Fraction_free(OWNED NULLABLE Fraction* frac);

/* Copying */
OWNED NULLABLE_WHEN(frac == NULL) Fraction* Fraction_copy(NULLABLE const Fraction* frac);

/* In-place simplification */
void Fraction_simplify(INOUT NONNULL Fraction* frac);
void Fraction_reduce(INOUT NONNULL Value* frac);

/* Arithmetic operations */
OWNED NONNULL Value* Fraction_add(NONNULL const Fraction* a, NONNULL const Value* b);
OWNED NONNULL Value* Fraction_sub(NONNULL const Fraction* a, NONNULL const Value* b);
OWNED NONNULL Value* Fraction_mul(NONNULL const Fraction* a, NONNULL const Value* b);
OWNED NONNULL Value* Fraction_div(NONNULL const Fraction* a, NONNULL const Value* b);
OWNED NONNULL Value* Fraction_mod(NONNULL const Fraction* a, NONNULL const Value* b);
OWNED NONNULL Value* Fraction_pow(NONNULL const Fraction* base, NONNULL const Value* exp);
OWNED NONNULL Value* Fraction_rpow(NONNULL const Fraction* exp, NONNULL const Value* base);

/* Comparison */
int Fraction_cmp(NONNULL const Fraction* a, NONNULL const Value* b);

/* Conversion */
double Fraction_asReal(NONNULL const Fraction* frac);

/* Printing */
OWNED NONNULL char* Fraction_repr(NONNULL const Fraction* frac, bool approx);
OWNED NONNULL char* Fraction_xml(NONNULL const Fraction* frac);

#endif /* SC_FRACTION_H */
