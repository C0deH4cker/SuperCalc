//
//  vector.h
//  SuperCalc
//
//  Created by Silas Schwarz on 11/16/13.
//  Copyright (c) 2013 Silas Schwarz. All rights reserved.
//

#ifndef SuperCalc_vector_h
#define SuperCalc_vector_h

#include "value.h"
#include "arglist.h"

// typedef struct vector_t Vector;

// #define Vector struct vector_t

//Vector {
//    long long count;
//    Value **args;
//};

 typedef ArgList Vector;

/*
 * Constructors
 */
// 'count' is required. If 'args' is NULL they will be created as 0 int values.
// All values in 'args' and are consumed, as is 'count'
Value *Vector_new(Value *count, Value **args);
Value *Vector_copy(Value *vector);
// The input vector is obviously consumed.
void Vector_free(Vector *vector);
Value *Vector_parse(const char **expr);

void Vector_register(Context *ctx);

Value *Vector_eval(Value *vector, Context *ctx);
char *Vector_repr(Value *vector);

/*
 * Common functions for use with vectors. No values are consumed.
 */
Value *Vector_scalar_multiply(Value *vector, Value *scalar, Context *ctx);
Value *Vector_scalar_divide(Value *vector, Value *scalar, Context *ctx);
Value *Vector_comp_multiply(Value *vector1, Value *vector2, Context *ctx);
Value *Vector_comp_divide(Value *vector1, Value *vector2, Context *ctx);
Value *Vector_comp_add(Value *vector1, Value *vector2, Context *ctx);
Value *Vector_comp_subtract(Value *vector1, Value *vector2, Context *ctx);

#endif
