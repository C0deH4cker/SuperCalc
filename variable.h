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


struct Variable {
	OWNED NULLABLE char* name;
	OWNED NONNULL Value* val;
};


/* Constructors */
/* Each of these methods consume its last argument */
OWNED NONNULL Variable* Variable_new(OWNED NULLABLE char* name, OWNED NONNULL Value* val);
OWNED NONNULL Variable* VarErr(OWNED NONNULL Error* err);

/* Destructor */
void Variable_free(OWNED NULLABLE Variable* var);

/* Copying */
OWNED NULLABLE_WHEN(var == NULL) Variable* Variable_copy(NULLABLE const Variable* var);

/* Evaluation */
OWNED NONNULL Value* Variable_eval(NONNULL const Variable* var, NONNULL const Context* ctx);

/* Variable accessing */
UNOWNED NULLABLE Variable* Variable_get(NONNULL const Context* ctx, NONNULL const char* name);
UNOWNED NULLABLE Variable* Variable_getAbove(NONNULL const Context* ctx, NONNULL const char* name);

/*
 This method basically frees the content of `dst` and moves
 the `src` value into `dst`.
*/
void Variable_update(INOUT UNOWNED NONNULL Variable* dst, IN OWNED NONNULL Value* src);

/* Printing */
OWNED NONNULL char* Variable_repr(NONNULL const Variable* var, bool pretty);
OWNED NONNULL char* Variable_wrap(NONNULL const Variable* var);
OWNED NONNULL char* Variable_verbose(NONNULL const Variable* var);
OWNED NONNULL char* Variable_xml(NONNULL const Variable* var);

#endif /* SC_VARIABLE_H */
