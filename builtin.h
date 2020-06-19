/*
  builtin.h
  SuperCalc

  Created by C0deH4cker on 11/8/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_BUILTIN_H
#define SC_BUILTIN_H

#include <stdbool.h>

typedef struct Builtin Builtin;
#include "context.h"
#include "arglist.h"
#include "value.h"
#include "generic.h"

typedef Value* (*builtin_eval_t)(NONNULL const Context*, NONNULL const ArgList*);

struct Builtin {
	OWNED NONNULL char* name;
	NONNULL builtin_eval_t evaluator;
	bool isFunction;
};

/* Constructor */
OWNED NONNULL Builtin* Builtin_new(OWNED NONNULL char* name, NONNULL builtin_eval_t evaluator, bool isFunction);

/* Destructor */
void Builtin_free(OWNED NULLABLE Builtin* blt);

/* Copying */
OWNED NULLABLE Builtin* Builtin_copy(NULLABLE const Builtin* blt);

/* Registration */
void Builtin_register(UNOWNED NONNULL Builtin* blt, UNOWNED NONNULL Context* ctx);

/* Evaluation */
OWNED NONNULL Value* Builtin_eval(
	NONNULL const Builtin* blt,
	NONNULL const Context* ctx,
	NONNULL_WHEN(blt->isFunction) const ArgList* arglist
);

/* Printing */
OWNED NONNULL char* Builtin_repr(NONNULL const Builtin* blt, bool pretty);
OWNED NONNULL char* Builtin_verbose(NONNULL const Builtin* blt, unsigned indent);
OWNED NONNULL char* Builtin_xml(NONNULL const Builtin* blt, unsigned indent);


#endif /* SC_BUILTIN_H */
