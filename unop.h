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


typedef enum {
	UN_FACT = 0
} UNTYPE;

struct UnOp {
	UNTYPE type;
	OWNED NONNULL Value* a;
};


/* Constructor */
/* This method consumes the `a` argument */
OWNED NONNULL UnOp* UnOp_new(UNTYPE type, OWNED NONNULL Value* a);

/* Destructor */
void UnOp_free(OWNED NULLABLE UnOp* term);

/* Copying */
OWNED NULLABLE_WHEN(term == NULL) UnOp* UnOp_copy(NULLABLE const UnOp* term);

/* Evaluation */
OWNED NONNULL Value* UnOp_eval(NONNULL const UnOp* term, NONNULL const Context* ctx);

/* Printing */
OWNED NONNULL char* UnOp_repr(NONNULL const UnOp* term, bool pretty);
OWNED NONNULL char* UnOp_wrap(NONNULL const UnOp* term);
OWNED NONNULL char* UnOp_verbose(NONNULL const UnOp* term, unsigned indent);
OWNED NONNULL char* UnOp_xml(NONNULL const UnOp* term, unsigned indent);

#endif /* SC_UNOP_H */
