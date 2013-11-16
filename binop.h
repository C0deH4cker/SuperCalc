/*
  binop.h
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_BINOP_H_
#define _SC_BINOP_H_


typedef struct BinOp BinOp;

#include "context.h"
#include "value.h"


typedef enum {
	BIN_UNK = -2,
	BIN_END = -1,
	BIN_ADD = 0,
	BIN_SUB,
	BIN_MUL,
	BIN_DIV,
	BIN_MOD,
	BIN_POW
} BINTYPE;

struct BinOp {
	BINTYPE type;
	Value* a;
	Value* b;
};

/* Constructor */
/* This method consumes both the `a` and `b` arguments */
BinOp* BinOp_new(BINTYPE type, Value* a, Value* b);

/* Destructor */
void BinOp_free(BinOp* node);

/* Copying */
BinOp* BinOp_copy(BinOp* node);

/* Evaluation */
Value* BinOp_eval(BinOp* node, Context* ctx);

/* Tokenizer */
BINTYPE BinOp_nextType(const char** expr, char sep, char end);

/* Operator precedence */
int BinOp_cmp(BINTYPE a, BINTYPE b);

/* Printing */
char* BinOp_verbose(BinOp* node, int indent);
char* BinOp_repr(BinOp* node);


#endif
