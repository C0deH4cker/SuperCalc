/*
  builtin.h
  SuperCalc

  Created by C0deH4cker on 11/8/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_BUILTIN_H_
#define _SC_BUILTIN_H_

#include <stdbool.h>

typedef struct Builtin Builtin;
#include "context.h"
#include "arglist.h"
#include "value.h"

typedef Value* (*builtin_eval_t)(const Context*, const ArgList*, bool);

struct Builtin {
	char* name;
	builtin_eval_t evaluator;
	bool isFunction;
};

/* Constructor */
Builtin* Builtin_new(const char* name, builtin_eval_t evaluator, bool isFunction);

/* Destructor */
void Builtin_free(Builtin* blt);

/* Copying */
Builtin* Builtin_copy(const Builtin* blt);

/* Registration */
void Builtin_register(Builtin* blt, Context* ctx);

/* Evaluation */
Value* Builtin_eval(const Builtin* blt, const Context* ctx, const ArgList* arglist, bool internal);

/* Printing */
char* Builtin_repr(const Builtin* blt, bool pretty);
char* Builtin_verbose(const Builtin* blt, unsigned indent);
char* Builtin_xml(const Builtin* blt, unsigned indent);


#endif
