/*
  variable.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_VARIABLE_H
#define SC_VARIABLE_H

#include <stdbool.h>

typedef struct Variable Variable;
#include "context.h"
#include "builtin.h"
#include "value.h"
#include "function.h"
#include "error.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct Variable {
	OWNED char* _Nullable name;
	OWNED Value* val;
};


DECL(Variable);

/* Constructors */
RETURNS_OWNED Variable* Variable_new(CONSUMED char* _Nullable name, CONSUMED Value* val);
RETURNS_OWNED Variable* VarErr(CONSUMED Error* err);

/* Destructor */
void Variable_free(OWNED Variable* _Nullable var);

/* Copying */
RETURNS_OWNED Variable* Variable_copy(const Variable* var);

/* Evaluation */
RETURNS_OWNED Value* Variable_eval(const Variable* var, const Context* ctx);

/* Variable accessing */
RETURNS_UNOWNED Variable* _Nullable Variable_get(const Context* ctx, const char* name);
RETURNS_UNOWNED Variable* _Nullable Variable_getAbove(const Context* ctx, const char* name);

/*
 This method basically frees the content of `dst` and moves
 the `src` value into `dst`.
*/
void Variable_update(INOUT UNOWNED Variable* dst, CONSUMED Value* src);

/* Printing */
RETURNS_OWNED char* Variable_repr(const Variable* var, bool pretty);
RETURNS_OWNED char* Variable_wrap(const Variable* var);
RETURNS_OWNED char* Variable_verbose(const Variable* var);
RETURNS_OWNED char* Variable_xml(const Variable* var);

METHOD_debugString(Variable);

ASSUME_NONNULL_END

#endif /* SC_VARIABLE_H */
