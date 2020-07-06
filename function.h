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


ASSUME_NONNULL_BEGIN

struct Function {
	unsigned argcount;
	OWNED char* _Nonnull * _Nullable_unless(argcount > 0) argnames;
	OWNED Value* _Nullable body;
};


DECL(Function);

/* Constructor */
RETURNS_OWNED Function* Function_new(
	unsigned argcount,
	CONSUMED char* _Nonnull * _Nullable_unless(argcount > 0) argnames,
	CONSUMED Value* _Nullable body
);

/* Destructor */
void Function_free(CONSUMED Function* _Nullable func);

/* Copying */
RETURNS_OWNED Function* Function_copy(const Function* func);

/* Evaluation */
RETURNS_OWNED Value* Function_eval(const Function* func, const Context* ctx, const ArgList* arglist);

/* Parsing */
RETURNS_OWNED Function* _Nullable Function_parseArgs(
	INOUT istring expr,
	char sep,
	char end,
	OUT RETURNS_OWNED Error* _Nullable_unless(return != NULL) * _Nonnull error
);

/* Printing */
RETURNS_OWNED char* Function_repr(const Function* func, const char* _Nullable name, bool pretty);
RETURNS_OWNED char* Function_wrap(const Function* func, const char* _Nullable name, bool top);
RETURNS_OWNED char* Function_verbose(const Function* func, unsigned indent);
RETURNS_OWNED char* Function_xml(const Function* func, unsigned indent);

METHOD_debugString(Function);

ASSUME_NONNULL_END

#endif /* SC_FUNCTION_H */
