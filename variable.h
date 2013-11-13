/*
  variable.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_VARIABLE_H_
#define _SC_VARIABLE_H_


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
		Builtin* blt;
		Value* val;
		Function* func;
		Error* err;
	};
};


/* Constructors */
Variable* VarBuiltin(const char* name, Builtin* blt);
Variable* VarValue(const char* name, Value* val);
Variable* VarFunc(const char* name, Function* func);
Variable* VarErr(Error* err);

/* Destructor */
void Variable_free(Variable* var);

/* Copying */
Variable* Variable_copy(Variable* var);

/* Evaluation */
Value* Variable_eval(const char* name, Context* ctx);

/* Variable accessing */
Variable* Variable_get(Context* ctx, const char* name);
void Variable_update(Variable* dst, Variable* src);

/* Printing */
char* Variable_verbose(Variable* var);
char* Variable_repr(Variable* var);

#endif
