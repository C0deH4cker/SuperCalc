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
#include "value.h"
#include "supercalc.h"
#include "context.h"
#include "generic.h"

struct Statement {
	OWNED NULLABLE char* assignee;
	OWNED NONNULL Value* val;
};


/* Constructor */
OWNED NONNULL Statement* Statement_new(IN OWNED NULLABLE char* assignee, IN OWNED NONNULL Value* val);

/* Destructor */
void Statement_free(IN OWNED NULLABLE Statement* stmt);

/* Parsing */
OWNED NONNULL Statement* Statement_parse(INOUT NONNULL const char** expr);

/* Error handling */
bool Statement_didError(NONNULL const Statement* stmt);

/* Evaluation */
OWNED NONNULL Value* Statement_eval(NONNULL const Statement* stmt, INOUT UNOWNED NONNULL Context* ctx);

/* Printing */
OWNED NONNULL char* Statement_repr(NONNULL const Statement* stmt, NONNULL const Context* ctx, bool pretty);
OWNED NONNULL char* Statement_wrap(NONNULL const Statement* stmt, NONNULL const Context* ctx);
OWNED NONNULL char* Statement_verbose(NONNULL const Statement* stmt, NONNULL const Context* ctx);
OWNED NONNULL char* Statement_xml(NONNULL const Statement* stmt, NONNULL const Context* ctx);
void Statement_print(NONNULL const Statement* stmt, NONNULL const SuperCalc* sc, VERBOSITY v);

#endif /* SC_STATEMENT_H */
