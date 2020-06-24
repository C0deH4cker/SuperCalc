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


ASSUME_NONNULL_BEGIN

struct ArgList {
	OWNED Value* _Nonnull * _Nullable_unless(count > 0) args;
	unsigned count;
};

/* Constructor */
RETURNS_OWNED ArgList* ArgList_new(unsigned count);

/* Destructor */
void ArgList_free(CONSUMED ArgList* _Nullable arglist);

/* Initializer */
RETURNS_OWNED ArgList* ArgList_create(unsigned count, /* OWNED Value* */...);
RETURNS_OWNED ArgList* ArgList_vcreate(unsigned count, va_list args);

/* Copying */
RETURNS_OWNED ArgList* ArgList_copy(const ArgList* arglist);

/* Evaluation */
RETURNS_OWNED ArgList* _Nullable ArgList_eval(
	const ArgList* arglist,
	const Context* ctx,
	OUT RETURNS_OWNED Error* _Nullable_unless(return == NULL) * _Nonnull err
);
RETURNS_OWNED double* _Nullable_unless(arglist->count > 0) ArgList_toReals(const ArgList* arglist, const Context* ctx);

/* Parsing */
RETURNS_OWNED ArgList* _Nullable ArgList_parse(
	INOUT istring expr,
	char sep,
	char end,
	parser_cb* cb,
	RETURNS_OWNED Error* _Nullable_unless(return == NULL) * _Nonnull err
);

/* Printing */
RETURNS_OWNED char* ArgList_repr(const ArgList* arglist, bool pretty);
RETURNS_OWNED char* ArgList_wrap(const ArgList* arglist);
RETURNS_OWNED char* ArgList_verbose(const ArgList* arglist, unsigned indent);
RETURNS_OWNED char* ArgList_xml(const ArgList* arglist, unsigned indent);

ASSUME_NONNULL_END

#endif /* SC_ARGLIST_H */
