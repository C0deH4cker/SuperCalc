/*
  function.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_FUNCTION_H_
#define _SC_FUNCTION_H_


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
Function* Function_new(unsigned argcount, char** argnames, Value* body);

/* Destructor */
void Function_free(Function* func);

/* Copying */
Function* Function_copy(Function* func);

/* Evaluation */
Value* Function_eval(Function* func, Context* ctx, ArgList* arglist);

/* Printing */
char* Function_verbose(Function* func);
char* Function_repr(Function* func);


#endif
