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


ASSUME_NONNULL_BEGIN

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
	OWNED Value* a;
	OWNED Value* _Nullable b;
};


DECL(BinOp);

/* Contains strings such as "multiply" for index BIN_MUL */
extern const char* _Nonnull binop_verb[BIN_COUNT];

/* Constructor */
RETURNS_OWNED BinOp* BinOp_new(BINTYPE type, CONSUMED Value* a, CONSUMED Value* _Nullable b);

/* Destructor */
void BinOp_free(CONSUMED BinOp* _Nullable node);

/* Copying */
RETURNS_OWNED BinOp* BinOp_copy(const BinOp* node);

/* Evaluation */
RETURNS_OWNED Value* BinOp_eval(INVARIANT(node->b != NULL) const BinOp* node, const Context* ctx);

/* Tokenizer */
BINTYPE BinOp_nextType(INOUT istring expr, char sep, char end);

/* Operator precedence */
int BinOp_cmp(BINTYPE a, BINTYPE b);

/* Printing */
RETURNS_OWNED char* BinOp_repr(INVARIANT(node->b != NULL) const BinOp* node, bool pretty);
RETURNS_OWNED char* BinOp_wrap(INVARIANT(node->b != NULL) const BinOp* node);
RETURNS_OWNED char* BinOp_verbose(INVARIANT(node->b != NULL) const BinOp* node, unsigned indent);
RETURNS_OWNED char* BinOp_xml(INVARIANT(node->b != NULL) const BinOp* node, unsigned indent);

METHOD_debugString(BinOp);

ASSUME_NONNULL_END

#endif /* SC_BINOP_H */
