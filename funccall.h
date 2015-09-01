/*
  funccall.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_FUNCCALL_H_
#define _SC_FUNCCALL_H_

#include <stdbool.h>

typedef struct FuncCall FuncCall;

#include "context.h"
#include "value.h"
#include "arglist.h"


struct FuncCall {
	Value* func;
	ArgList* arglist;
};


/* Constructor */
/* This method consumes both the `func` and `arglist` arguments */
FuncCall* FuncCall_new(Value* func, ArgList* arglist);
/* Used to create specific calls like "sqrt". `arglist` is consumed */
FuncCall* FuncCall_create(const char* name, ArgList* arglist);

/* Destructor */
void FuncCall_free(FuncCall* call);

/* Copying */
FuncCall* FuncCall_copy(const FuncCall* call);

/* Evaluation */
Value* FuncCall_eval(const FuncCall* call, const Context* ctx);

/* Printing */
char* FuncCall_repr(const FuncCall* call, bool pretty);
char* FuncCall_verbose(const FuncCall* call, unsigned indent);
char* FuncCall_xml(const FuncCall* call, unsigned indent);

#endif
