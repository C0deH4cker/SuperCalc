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


struct FuncCall {
	OWNED NONNULL Value* func;
	OWNED NONNULL ArgList* arglist;
};


/* Constructor */
OWNED NONNULL FuncCall* FuncCall_new(OWNED NONNULL Value* func, OWNED NONNULL ArgList* arglist);
/* Used to create specific calls like "sqrt" */
OWNED NONNULL FuncCall* FuncCall_create(OWNED NONNULL char* name, OWNED NONNULL ArgList* arglist);

/* Destructor */
void FuncCall_free(OWNED NULLABLE FuncCall* call);

/* Copying */
OWNED NULLABLE_WHEN(call == NULL) FuncCall* FuncCall_copy(NULLABLE const FuncCall* call);

void FuncCall_setScope(UNOWNED NONNULL FuncCall* call, NONNULL const Context* ctx);

/* Evaluation */
OWNED NONNULL Value* FuncCall_eval(NULLABLE const FuncCall* call, NONNULL const Context* ctx);

/* Printing */
OWNED NONNULL char* FuncCall_repr(NONNULL const FuncCall* call, bool pretty);
OWNED NONNULL char* FuncCall_wrap(NONNULL const FuncCall* call);
OWNED NONNULL char* FuncCall_verbose(NONNULL const FuncCall* call, unsigned indent);
OWNED NONNULL char* FuncCall_xml(NONNULL const FuncCall* call, unsigned indent);

#endif /* SC_FUNCCALL_H */
