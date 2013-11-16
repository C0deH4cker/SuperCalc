/*
  unop.h
  SuperCalc

  Created by C0deH4cker on 11/6/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_UNOP_H_
#define _SC_UNOP_H_


typedef struct UnOp UnOp;

#include "context.h"
#include "value.h"


typedef enum {
	UN_FACT = 0
} UNTYPE;

struct UnOp {
	UNTYPE type;
	Value* a;
};


/* Constructor */
/* This method consumes the `a` argument */
UnOp* UnOp_new(UNTYPE type, Value* a);

/* Destructor */
void UnOp_free(UnOp* term);

/* Copying */
UnOp* UnOp_copy(UnOp* term);

/* Evaluation */
Value* UnOp_eval(UnOp* term, Context* ctx);

/* Printing */
char* UnOp_verbose(UnOp* term, int indent);
char* UnOp_repr(UnOp* term);

#endif
