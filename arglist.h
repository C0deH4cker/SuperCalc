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
	unsigned count;
	Value** args;
};

/* Constructor */
ArgList* ArgList_new(unsigned count);

/* Destructor */
void ArgList_free(ArgList* arglist);

/* Initializer */
ArgList* ArgList_create(unsigned count, /* Value* */...);
ArgList* ArgList_vcreate(unsigned count, va_list args);

/* Copying */
ArgList* ArgList_copy(ArgList* arglist);

/* Evaluation */
ArgList* ArgList_eval(ArgList* arglist, Context* ctx);
double* ArgList_toReals(ArgList* arglist, Context* ctx);

/* Parsing */
ArgList* ArgList_parse(const char** expr, char sep, char end);

/* Printing */
char* ArgList_verbose(ArgList* arglist, int indent);
char* ArgList_repr(ArgList* arglist, bool pretty);


#endif
