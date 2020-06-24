/*
  funccall.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_FUNCCALL_H
#define SC_FUNCCALL_H

#include <stdbool.h>

typedef struct FuncCall FuncCall;

#include "context.h"
#include "value.h"
#include "arglist.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct FuncCall {
	OWNED Value* func;
	OWNED ArgList* arglist;
};


/* Constructor */
RETURNS_OWNED FuncCall* FuncCall_new(CONSUMED Value* func, CONSUMED ArgList* arglist);
/* Used to create specific calls like "sqrt" */
RETURNS_OWNED FuncCall* FuncCall_create(CONSUMED char* name, CONSUMED ArgList* arglist);

/* Destructor */
void FuncCall_free(CONSUMED FuncCall* _Nullable call);

/* Copying */
RETURNS_OWNED FuncCall* FuncCall_copy(const FuncCall* call);

/* Evaluation */
RETURNS_OWNED Value* FuncCall_eval(const FuncCall* _Nullable call, const Context* ctx);

/* Printing */
RETURNS_OWNED char* FuncCall_repr(const FuncCall* call, bool pretty);
RETURNS_OWNED char* FuncCall_wrap(const FuncCall* call);
RETURNS_OWNED char* FuncCall_verbose(const FuncCall* call, unsigned indent);
RETURNS_OWNED char* FuncCall_xml(const FuncCall* call, unsigned indent);

ASSUME_NONNULL_END

#endif /* SC_FUNCCALL_H */
