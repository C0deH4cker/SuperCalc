/*
  statement.h
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_STATEMENT_H
#define SC_STATEMENT_H

#include <stdio.h>
#include <stdbool.h>

typedef struct Statement Statement;
#include "variable.h"
#include "value.h"
#include "supercalc.h"
#include "context.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct Statement {
	OWNED Variable* var;
};


DECL(Statement);

/* Constructor */
/*
 This method consumes the `var` argument, which is an unusual variable:
  - Its name is NULL unless there is an assignment
  - Its value is not simplified. It will store the entire tree
*/
RETURNS_OWNED Statement* Statement_new(CONSUMED Variable* var);

/* Destructor */
void Statement_free(CONSUMED Statement* _Nullable stmt);

/* Parsing */
RETURNS_OWNED Statement* Statement_parse(INOUT istring expr);

/* Error handling */
bool Statement_didError(const Statement* stmt);

/* Evaluation */
RETURNS_OWNED Value* Statement_eval(const Statement* stmt, INOUT UNOWNED Context* ctx, VERBOSITY v);

/* Printing */
RETURNS_OWNED char* Statement_repr(const Statement* stmt, const Context* ctx, bool pretty);
RETURNS_OWNED char* Statement_wrap(const Statement* stmt, const Context* ctx);
RETURNS_OWNED char* Statement_verbose(const Statement* stmt, const Context* ctx);
RETURNS_OWNED char* Statement_xml(const Statement* stmt, const Context* ctx);
void Statement_print(const Statement* stmt, const SuperCalc* sc, VERBOSITY v);

METHOD_debugString(Statement);

ASSUME_NONNULL_END

#endif /* SC_STATEMENT_H */
