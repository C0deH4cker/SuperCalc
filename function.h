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


struct Function {
	unsigned argcount;
	char** argnames;
	Value* body;
};


/* Constructor */
/* This method consumes both the `argnames` and `body` arguments */
Function* Function_new(unsigned argcount, char** argnames, Value* body);

/* Destructor */
void Function_free(Function* func);

/* Copying */
Function* Function_copy(const Function* func);

/* Evaluation */
Value* Function_eval(const Function* func, const Context* ctx, const ArgList* arglist);

/* Printing */
char* Function_repr(const Function* func, bool pretty);
char* Function_wrap(const Function* func);
char* Function_verbose(const Function* func);
char* Function_xml(const Function* func, unsigned indent);

#endif /* SC_FUNCTION_H */
