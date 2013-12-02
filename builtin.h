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

typedef Value* (*builtin_eval_t)(Context*, ArgList*, bool);

struct Builtin {
	char* name;
	builtin_eval_t evaluator;
};

/* Constructor */
Builtin* Builtin_new(const char* name, builtin_eval_t evaluator);

/* Destructor */
void Builtin_free(Builtin* blt);

/* Copying */
Builtin* Builtin_copy(Builtin* blt);

/* Registration */
void Builtin_register(Builtin* blt, Context* ctx);

/* Evaluation */
Value* Builtin_eval(Builtin* blt, Context* ctx, ArgList* arglist, bool internal);

/* Printing */
char* Builtin_verbose(Builtin* blt, int indent);
char* Builtin_repr(Builtin* blt);


#endif
