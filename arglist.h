/*
  arglist.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_ARGLIST_H
#define SC_ARGLIST_H

#include <stdarg.h>
#include <stdbool.h>

typedef struct ArgList ArgList;
#include "value.h"
#include "context.h"
#include "generic.h"

struct ArgList {
	OWNED NONNULL Value* OWNED NONNULL_WHEN(count > 0)* args;
	unsigned count;
};

/* Constructor */
OWNED NONNULL ArgList* ArgList_new(unsigned count);

/* Destructor */
void ArgList_free(OWNED NULLABLE ArgList* arglist);

/* Initializer */
OWNED NONNULL ArgList* ArgList_create(unsigned count, /* OWNED NONNULL Value* */...);
OWNED NONNULL ArgList* ArgList_vcreate(unsigned count, va_list args);

/* Copying */
OWNED NONNULL_WHEN(arglist != NULL) ArgList* ArgList_copy(NULLABLE const ArgList* arglist);

/* Evaluation */
OWNED NONNULL ArgList* ArgList_eval(NONNULL const ArgList* arglist, NONNULL const Context* ctx);
OWNED NONNULL_WHEN(arglist->count > 0) double* ArgList_toReals(NONNULL const ArgList* arglist, NONNULL const Context* ctx);

/* Parsing */
OWNED NULLABLE ArgList* ArgList_parse(
	NONNULL const char** expr,
	char sep,
	char end,
	NONNULL parser_cb* cb,
	OWNED NONNULL_WHEN(return == NULL) Error* NONNULL* err
);

/* Printing */
OWNED NONNULL char* ArgList_repr(NONNULL const ArgList* arglist, bool pretty);
OWNED NONNULL char* ArgList_wrap(NONNULL const ArgList* arglist);
OWNED NONNULL char* ArgList_verbose(NONNULL const ArgList* arglist, unsigned indent);
OWNED NONNULL char* ArgList_xml(NONNULL const ArgList* arglist, unsigned indent);

#endif /* SC_ARGLIST_H */
