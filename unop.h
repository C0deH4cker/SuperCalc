/*
  unop.h
  SuperCalc

  Created by C0deH4cker on 11/6/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_UNOP_H_
#define _SC_UNOP_H_

#include <stdbool.h>

typedef struct UnOp UnOp;

#include "context.h"
#include "value.h"
#include "generic.h"


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
UnOp* UnOp_copy(const UnOp* term);

/* Evaluation */
Value* UnOp_eval(const UnOp* term, const Context* ctx);

/* Printing */
char* UnOp_verbose(const UnOp* term, int indent);
char* UnOp_repr(const UnOp* term, bool pretty);

#endif
