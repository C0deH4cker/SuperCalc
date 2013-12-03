/*
  variable.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_VARIABLE_H_
#define _SC_VARIABLE_H_

#include <stdbool.h>

typedef struct Variable Variable;
#include "context.h"
#include "builtin.h"
#include "value.h"
#include "function.h"
#include "error.h"


typedef enum {
	VAR_ERR = -1,
	VAR_BUILTIN = 0,
	VAR_VALUE,
	VAR_FUNC
} VARTYPE;

struct Variable {
	VARTYPE type;
	char* name;
	union {
		Error* err;
		Builtin* blt;
		Value* val;
		Function* func;
	};
};


/* Constructors */
/* Each of these methods consume its last argument */
Variable* VarErr(Error* err);
Variable* VarBuiltin(const char* name, Builtin* blt);
Variable* VarValue(const char* name, Value* val);
Variable* VarFunc(const char* name, Function* func);

/* Destructor */
void Variable_free(Variable* var);

/* Copying */
Variable* Variable_copy(Variable* var);

/* Evaluation */
Value* Variable_eval(Variable* var, Context* ctx);
Value* Variable_coerce(Variable* var, Context* ctx);

/* Variable accessing */
Variable* Variable_get(Context* ctx, const char* name);
Variable* Variable_getAbove(Context* ctx, const char* name);

/*
 This method basically frees the content of `dst`, moves
 the content of `src` into dst, then frees `src`, so
 `src` is consumed.
*/
void Variable_update(Variable* dst, Variable* src);

/* Printing */
char* Variable_verbose(Variable* var);
char* Variable_repr(Variable* var, bool pretty);

#endif
