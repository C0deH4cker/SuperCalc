/*
  unop.h
  SuperCalc

  Created by C0deH4cker on 11/6/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_UNOP_H
#define SC_UNOP_H

#include <stdbool.h>

typedef struct UnOp UnOp;

#include "context.h"
#include "value.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

typedef enum {
	UN_FACT = 0
} UNTYPE;

struct UnOp {
	UNTYPE type;
	OWNED Value* _Nonnull a;
};


/* Constructor */
RETURNS_OWNED UnOp* UnOp_new(UNTYPE type, CONSUMED Value* a);

/* Destructor */
void UnOp_free(CONSUMED UnOp* _Nullable term);

/* Copying */
RETURNS_OWNED UnOp* UnOp_copy(const UnOp* term);

/* Evaluation */
RETURNS_OWNED Value* UnOp_eval(const UnOp* term, const Context* ctx);

/* Printing */
RETURNS_OWNED char* UnOp_repr(const UnOp* term, bool pretty);
RETURNS_OWNED char* UnOp_wrap(const UnOp* term);
RETURNS_OWNED char* UnOp_verbose(const UnOp* term, unsigned indent);
RETURNS_OWNED char* UnOp_xml(const UnOp* term, unsigned indent);

ASSUME_NONNULL_END

#endif /* SC_UNOP_H */
