/*
  vector.h
  SuperCalc

  Created by Silas Schwarz on 11/16/13.
  Copyright (c) 2013 C0deH4cker and Silas Schwarz. All rights reserved.
*/

#ifndef SC_VECTOR_H
#define SC_VECTOR_H

#include <stdarg.h>
#include <stdbool.h>

typedef struct Vector Vector;
#include "value.h"
#include "arglist.h"
#include "context.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct Vector {
	OWNED ArgList* vals;
};


/* Constructor */
RETURNS_OWNED Vector* Vector_new(CONSUMED ArgList* vals);
RETURNS_OWNED Vector* Vector_create(INVARIANT(count >= 1) unsigned count, /* Value* */...);
RETURNS_OWNED Vector* Vector_vcreate(INVARIANT(count >= 1) unsigned count, va_list args);

/* Destructor */
void Vector_free(CONSUMED Vector* _Nullable vec);

/* Copying */
RETURNS_OWNED Vector* Vector_copy(const Vector* vec);

/* Parsing */
RETURNS_OWNED Value* Vector_parse(INOUT istring expr, parser_cb* cb);

/* Evaluation */
RETURNS_OWNED Value* Vector_eval(const Vector* vec, const Context* ctx);

/* Arithmetic */
RETURNS_OWNED Value* Vector_add(const Vector* vec, const Value* other, const Context* ctx);
RETURNS_OWNED Value* Vector_sub(const Vector* vec, const Value* other, const Context* ctx);
RETURNS_OWNED Value* Vector_rsub(const Vector* vec, const Value* scalar, const Context* ctx);
RETURNS_OWNED Value* Vector_mul(const Vector* vec, const Value* other, const Context* ctx);
RETURNS_OWNED Value* Vector_div(const Vector* vec, const Value* other, const Context* ctx);
RETURNS_OWNED Value* Vector_rdiv(const Vector* vec, const Value* scalar, const Context* ctx);
RETURNS_OWNED Value* Vector_pow(const Vector* vec, const Value* other, const Context* ctx);
RETURNS_OWNED Value* Vector_rpow(const Vector* vec, const Value* scalar, const Context* ctx);
RETURNS_OWNED Value* Vector_dot(const Vector* vector1, const Vector* vector2, const Context* ctx);
RETURNS_OWNED Value* Vector_cross(const Vector* vector1, const Vector* vector2, const Context* ctx);
RETURNS_OWNED Value* Vector_magnitude(const Vector* vec, const Context* ctx);
RETURNS_OWNED Value* Vector_normalize(const Vector* vec, const Context* ctx);

/* Access Values */
RETURNS_OWNED Value* Vector_elem(const Vector* vec, const Value* index, const Context* ctx);

/* Printing */
RETURNS_OWNED char* Vector_repr(const Vector* vec, bool pretty);
RETURNS_OWNED char* Vector_wrap(const Vector* vec);
RETURNS_OWNED char* Vector_verbose(const Vector* vec, unsigned indent);
RETURNS_OWNED char* Vector_xml(const Vector* vec, unsigned indent);

ASSUME_NONNULL_END

#endif /* SC_VECTOR_H */
