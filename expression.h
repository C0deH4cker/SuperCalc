/*
  expression.h
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_EXPRESSION_H_
#define _SC_EXPRESSION_H_

typedef struct Expression Expression;
#include "variable.h"
#include "value.h"

struct Expression {
	Variable* var;
};


/* Constructor */
Expression* Expression_new(Variable* var);

/* Destructor */
void Expression_free(Expression* expr);

/* Parsing */
Expression* Expression_parse(const char** expr);

/* Error handling */
bool Expression_didError(Expression* expr);

/* Evaluation */
Value* Expression_eval(Expression* expr, Context* ctx);

/* Printing */
char* Expression_verbose(Expression* expr, int indent);
char* Expression_repr(Expression* expr);
void Expression_print(Expression* expr, int verbosity);

#endif
