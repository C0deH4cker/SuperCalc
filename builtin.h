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


ASSUME_NONNULL_BEGIN

typedef Value* _Nonnull (*builtin_eval_t)(const Context* _Nonnull, const ArgList* _Nonnull, bool);

struct Builtin {
	OWNED char* name;
	builtin_eval_t evaluator;
	bool isFunction;
};


DECL(Builtin);

/* Constructor */
RETURNS_OWNED Builtin* Builtin_new(CONSUMED char* name, builtin_eval_t evaluator, bool isFunction);

/* Destructor */
void Builtin_free(CONSUMED Builtin* _Nullable blt);

/* Copying */
RETURNS_OWNED Builtin* Builtin_copy(const Builtin* blt);

/* Registration */
void Builtin_register(IN Builtin* blt, IN Context* ctx);

/* Evaluation */
RETURNS_OWNED Value* Builtin_eval(
	const Builtin* blt,
	const Context* ctx,
	const ArgList* arglist,
	bool internal
);

/* Printing */
RETURNS_OWNED char* Builtin_repr(const Builtin* blt, bool pretty);
RETURNS_OWNED char* Builtin_verbose(const Builtin* blt, unsigned indent);
RETURNS_OWNED char* Builtin_xml(const Builtin* blt, unsigned indent);

METHOD_debugString(Builtin);

ASSUME_NONNULL_END

#endif /* SC_BUILTIN_H */
