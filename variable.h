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
	UNOWNED NULLABLE Value* val;
	NULLABLE const Context* scope;
};


/* Constructors */
OWNED NONNULL Variable* Variable_new(OWNED NULLABLE char* name);

/* Destructor */
void Variable_free(IN OWNED NULLABLE Variable* var);

/* Copying */
OWNED NULLABLE_WHEN(var == NULL) Variable* Variable_copy(NULLABLE const Variable* var);

/* Override the context that will be used when this variable is evaluated */
void Variable_setScope(INOUT UNOWNED NONNULL Variable* var, NULLABLE const Context* scope);

/* Get the value this variable stores */
OWNED NONNULL Value* Variable_lookup(INOUT UNOWNED NONNULL Variable* var, NONNULL const Context* ctx);

/*
 This method basically frees the content of `dst` and moves
 the `src` value into `dst`.
*/
void Variable_update(INOUT UNOWNED NONNULL Variable* dst, IN OWNED NONNULL Value* src);

/* Printing */
OWNED NONNULL char* Variable_repr(NONNULL const char* name, NULLABLE const Value* val, bool pretty);
OWNED NONNULL char* Variable_wrap(NONNULL const char* name, NULLABLE const Value* val);
OWNED NONNULL char* Variable_verbose(NONNULL const char* name, NULLABLE const Value* val);
OWNED NONNULL char* Variable_xml(NONNULL const char* name, NULLABLE const Value* val);

#endif /* SC_VARIABLE_H */
