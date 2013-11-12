/*
  funccall.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_FUNCCALL_H_
#define _SC_FUNCCALL_H_


typedef struct FuncCall FuncCall;

#include "context.h"
#include "value.h"
#include "variable.h"
#include "arglist.h"


struct FuncCall {
	char* name;
	ArgList* arglist;
};


/* Constructor */
FuncCall* FuncCall_new(const char* name, ArgList* arglist);

/* Destructor */
void FuncCall_free(FuncCall* call);

/* Copying */
FuncCall* FuncCall_copy(FuncCall* call);

/* Evaluation */
Value* FuncCall_eval(FuncCall* call, Context* ctx);

/* Printing */
char* FuncCall_verbose(FuncCall* call, int indent);
char* FuncCall_repr(FuncCall* call);

#endif
