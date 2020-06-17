/*
  binop.h
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_BINOP_H
#define SC_BINOP_H

#include <stdbool.h>

typedef struct BinOp BinOp;

#include "context.h"
#include "value.h"
#include "generic.h"


typedef enum {
	BIN_UNK = -2,
	BIN_END = -1,
	BIN_ADD = 0,
	BIN_SUB,
	BIN_MUL,
	BIN_DIV,
	BIN_MOD,
	BIN_POW,
	BIN_AFTERMAX /* pseudo type with highest precedence */
} BINTYPE;

#define BIN_COUNT (BIN_AFTERMAX)

struct BinOp {
	INVARIANT(type >= BIN_ADD) BINTYPE type;
	OWNED NONNULL Value* a;
	OWNED NULLABLE Value* b;
};

/* Contains strings such as "multiply" for index BIN_MUL */
extern const char* binop_verb[BIN_COUNT];

/* Constructor */
/* This method consumes both the `a` and `b` arguments */
BinOp* BinOp_new(BINTYPE type, OWNED NONNULL Value* a, OWNED NULLABLE Value* b);

/* Destructor */
void BinOp_free(OWNED NULLABLE BinOp* node);

/* Copying */
OWNED NULLABLE_WHEN(node == NULL) BinOp* BinOp_copy(NULLABLE const BinOp* node);

/* Evaluation */
OWNED NONNULL Value* BinOp_eval(NONNULL INVARIANT(node->b != NULL) const BinOp* node, NONNULL const Context* ctx);

/* Tokenizer */
BINTYPE BinOp_nextType(INOUT NONNULL const char** expr, char sep, char end);

/* Operator precedence */
int BinOp_cmp(BINTYPE a, BINTYPE b);

/* Printing */
OWNED NONNULL char* BinOp_repr(NONNULL INVARIANT(node->b != NULL) const BinOp* node, bool pretty);
OWNED NONNULL char* BinOp_wrap(NONNULL INVARIANT(node->b != NULL) const BinOp* node);
OWNED NONNULL char* BinOp_verbose(NONNULL INVARIANT(node->b != NULL) const BinOp* node, unsigned indent);
OWNED NONNULL char* BinOp_xml(NONNULL INVARIANT(node->b != NULL) const BinOp* node, unsigned indent);

#endif /* SC_BINOP_H */
