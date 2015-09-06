/*
  binop.h
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_BINOP_H_
#define _SC_BINOP_H_

#include <stdbool.h>

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
	BIN_POW,
	BIN_HIGHEST /* pseudo type with highest precedence */
} BINTYPE;

#define BIN_COUNT (BIN_HIGHEST)

struct BinOp {
	BINTYPE type;
	Value* a;
	Value* b;
};

/* Contains strings such as "multiply" for index BIN_ADD */
extern const char* binop_verb[BIN_COUNT];

/* Constructor */
/* This method consumes both the `a` and `b` arguments */
BinOp* BinOp_new(BINTYPE type, Value* a, Value* b);

/* Destructor */
void BinOp_free(BinOp* node);

/* Copying */
BinOp* BinOp_copy(const BinOp* node);

/* Evaluation */
Value* BinOp_eval(const BinOp* node, const Context* ctx);

/* Tokenizer */
BINTYPE BinOp_nextType(const char** expr, char sep, char end);

/* Operator precedence */
int BinOp_cmp(BINTYPE a, BINTYPE b);

/* Printing */
char* BinOp_repr(const BinOp* node, bool pretty);
char* BinOp_wrap(const BinOp* node);
char* BinOp_verbose(const BinOp* node, unsigned indent);
char* BinOp_xml(const BinOp* node, unsigned indent);

#endif
