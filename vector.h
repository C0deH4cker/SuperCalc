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


struct Vector {
	OWNED NONNULL ArgList* vals;
};


/* Constructor */
OWNED NONNULL Vector* Vector_new(OWNED NONNULL ArgList* vals);
OWNED NONNULL Vector* Vector_create(INVARIANT(count >= 1) unsigned count, /* Value* */...);
OWNED NONNULL Vector* Vector_vcreate(INVARIANT(count >= 1) unsigned count, va_list args);

/* Destructor */
void Vector_free(OWNED NULLABLE Vector* vec);

/* Copying */
OWNED NONNULL_WHEN(vec == NULL) Vector* Vector_copy(NULLABLE const Vector* vec);

void Vector_setScope(UNOWNED NONNULL Vector* vec, NONNULL const Context* ctx);

/* Parsing */
OWNED NONNULL Value* Vector_parse(INOUT NONNULL const char** expr, NONNULL parser_cb* cb);

/* Evaluation */
OWNED NONNULL Value* Vector_eval(NONNULL const Vector* vec, NONNULL const Context* ctx);

/* Arithmetic */
OWNED NONNULL Value* Vector_add(NONNULL const Vector* vec, NONNULL const Value* other, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_sub(NONNULL const Vector* vec, NONNULL const Value* other, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_rsub(NONNULL const Vector* vec, NONNULL const Value* scalar, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_mul(NONNULL const Vector* vec, NONNULL const Value* other, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_div(NONNULL const Vector* vec, NONNULL const Value* other, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_rdiv(NONNULL const Vector* vec, NONNULL const Value* scalar, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_pow(NONNULL const Vector* vec, NONNULL const Value* other, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_rpow(NONNULL const Vector* vec, NONNULL const Value* scalar, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_dot(NONNULL const Vector* vector1, NONNULL const Vector* vector2, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_cross(NONNULL const Vector* vector1, NONNULL const Vector* vector2, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_magnitude(NONNULL const Vector* vec, NONNULL const Context* ctx);
OWNED NONNULL Value* Vector_normalize(NONNULL const Vector* vec, NONNULL const Context* ctx);

/* Access Values */
OWNED NONNULL Value* Vector_elem(NONNULL const Vector* vec, NONNULL const Value* index, NONNULL const Context* ctx);

/* Printing */
OWNED NONNULL char* Vector_repr(NONNULL const Vector* vec, bool pretty);
OWNED NONNULL char* Vector_wrap(NONNULL const Vector* vec);
OWNED NONNULL char* Vector_verbose(NONNULL const Vector* vec, unsigned indent);
OWNED NONNULL char* Vector_xml(NONNULL const Vector* vec, unsigned indent);

#endif /* SC_VECTOR_H */
