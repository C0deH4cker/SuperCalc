/*
  function.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_FUNCTION_H
#define SC_FUNCTION_H

#include <stdbool.h>

typedef struct Function Function;

#include "context.h"
#include "arglist.h"
#include "value.h"
#include "generic.h"


struct Function {
	unsigned argcount;
	OWNED NONNULL char* OWNED NONNULL_WHEN(argcount > 0)* argnames;
	OWNED NONNULL Value* body;
};


/* Constructor */
/* This method consumes both the `argnames` and `body` arguments */
OWNED NONNULL Function* Function_new(
	unsigned argcount,
	OWNED NONNULL char* OWNED NONNULL_WHEN(argcount > 0)* argnames,
	OWNED NULLABLE Value* body
);

/* Destructor */
void Function_free(OWNED NULLABLE Function* func);

/* Copying */
OWNED NULLABLE_WHEN(func == NULL) Function* Function_copy(NULLABLE const Function* func);

/* Evaluation */
OWNED NONNULL Value* Function_eval(NONNULL const Function* func, NONNULL const Context* ctx, NONNULL const ArgList* arglist);

/* Parsing */
OWNED NULLABLE Function* Function_parseArgs(
	INOUT NONNULL const char** expr,
	char sep,
	char end,
	OUT OWNED NONNULL_WHEN(return == NULL) Error* NONNULL* error
);

/* Printing */
OWNED NONNULL char* Function_repr(NONNULL const Function* func, NULLABLE const char* name, bool pretty);
OWNED NONNULL char* Function_wrap(NONNULL const Function* func, NULLABLE const char* name, bool top);
OWNED NONNULL char* Function_verbose(NONNULL const Function* func, unsigned indent);
OWNED NONNULL char* Function_xml(NONNULL const Function* func, unsigned indent);

#endif /* SC_FUNCTION_H */
