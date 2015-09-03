/*
  arglist.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_ARGLIST_H_
#define _SC_ARGLIST_H_

#include <stdarg.h>
#include <stdbool.h>

typedef struct ArgList ArgList;
#include "value.h"
#include "context.h"

struct ArgList {
	Value** args;
	unsigned count;
};

/* Constructor */
ArgList* ArgList_new(unsigned count);

/* Destructor */
void ArgList_free(ArgList* arglist);

/* Initializer */
ArgList* ArgList_create(unsigned count, /* Value* */...);
ArgList* ArgList_vcreate(unsigned count, va_list args);

/* Copying */
ArgList* ArgList_copy(const ArgList* arglist);

/* Evaluation */
ArgList* ArgList_eval(const ArgList* arglist, const Context* ctx);
double* ArgList_toReals(const ArgList* arglist, const Context* ctx);

/* Parsing */
ArgList* ArgList_parse(const char** expr, char sep, char end, parser_cb* cb);

/* Printing */
char* ArgList_repr(const ArgList* arglist, bool pretty);
char* ArgList_wrap(const ArgList* arglist);
char* ArgList_verbose(const ArgList* arglist, unsigned indent);
char* ArgList_xml(const ArgList* arglist, unsigned indent);

#endif
