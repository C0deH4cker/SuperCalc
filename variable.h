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


struct Variable {
	char* name;
	Value* val;
};


/* Constructors */
/* Each of these methods consume its last argument */
Variable* Variable_new(char* name, Value* val);
Variable* VarErr(Error* err);

/* Destructor */
void Variable_free(Variable* var);

/* Copying */
Variable* Variable_copy(const Variable* var);

/* Evaluation */
Value* Variable_eval(const Variable* var, const Context* ctx);

/* Variable accessing */
Variable* Variable_get(const Context* ctx, const char* name);
Variable* Variable_getAbove(const Context* ctx, const char* name);

/*
 This method basically frees the content of `dst`, moves
 the content of `src` into dst, then frees `src`, so
 `src` is consumed.
*/
void Variable_update(Variable* dst, Variable* src);

/* Printing */
char* Variable_repr(const Variable* var, bool pretty);
char* Variable_wrap(const Variable* var);
char* Variable_verbose(const Variable* var);
char* Variable_xml(const Variable* var);

#endif /* SC_VARIABLE_H */
