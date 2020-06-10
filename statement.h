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

struct Statement {
	Variable* var;
};


/* Constructor */
/*
 This method consumes the `var` argument, which is an unusual variable:
  - Its name is NULL unless there is an assignment
  - Its value is not simplified. It will store the entire tree
*/
Statement* Statement_new(Variable* var);

/* Destructor */
void Statement_free(Statement* stmt);

/* Parsing */
Statement* Statement_parse(const char** expr);

/* Error handling */
bool Statement_didError(const Statement* stmt);

/* Evaluation */
Value* Statement_eval(const Statement* stmt, Context* ctx, VERBOSITY v);

/* Printing */
char* Statement_repr(const Statement* stmt, const Context* ctx, bool pretty);
char* Statement_wrap(const Statement* stmt, const Context* ctx);
char* Statement_verbose(const Statement* stmt, const Context* ctx);
char* Statement_xml(const Statement* stmt, const Context* ctx);
void Statement_print(const Statement* stmt, const SuperCalc* sc, VERBOSITY v);

#endif /* SC_STATEMENT_H */
