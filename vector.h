/*
  vector.h
  SuperCalc

  Created by Silas Schwarz on 11/16/13.
  Copyright (c) 2013 C0deH4cker and Silas Schwarz. All rights reserved.
*/

#ifndef _SC_VECTOR_H_
#define _SC_VECTOR_H_

#include <stdarg.h>

typedef struct Vector Vector;
#include "value.h"
#include "arglist.h"
#include "context.h"


struct Vector {
	ArgList* vals;
};


/* Constructor */
Vector* Vector_new(ArgList* vals);
Vector* Vector_create(unsigned count, /* Value* */...);
Vector* Vector_vcreate(unsigned count, va_list args);

/* Destructor */
void Vector_free(Vector* vec);

/* Copying */
Vector* Vector_copy(const Vector* vec);

/* Parsing */
Value* Vector_parse(const char** expr, parser_cb* cb);

/* Evaluation */
Value* Vector_eval(const Vector* vec, const Context* ctx);

/* Arithmetic */
Value* Vector_add(const Vector* vec, const Value* other, const Context* ctx);
Value* Vector_sub(const Vector* vec, const Value* other, const Context* ctx);
Value* Vector_rsub(const Vector* vec, const Value* scalar, const Context* ctx);
Value* Vector_mul(const Vector* vec, const Value* other, const Context* ctx);
Value* Vector_div(const Vector* vec, const Value* other, const Context* ctx);
Value* Vector_rdiv(const Vector* vec, const Value* scalar, const Context* ctx);
Value* Vector_pow(const Vector* vec, const Value* other, const Context* ctx);
Value* Vector_rpow(const Vector* vec, const Value* scalar, const Context* ctx);
Value* Vector_dot(const Vector* vector1, const Vector* vector2, const Context* ctx);
Value* Vector_cross(const Vector* vector1, const Vector* vector2, const Context* ctx);
Value* Vector_magnitude(const Vector* vec, const Context* ctx);
Value* Vector_normalize(const Vector* vec, const Context* ctx);

/* Access Values */
Value* Vector_elem(const Vector* vec, const Value* index, const Context* ctx);

/* Printing */
char* Vector_repr(const Vector* vec);
char* Vector_verbose(const Vector* vec, unsigned indent);
char* Vector_xml(const Vector* vec, unsigned indent);

#endif
