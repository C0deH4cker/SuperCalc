/*
  expression.h
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_EXPRESSION_H_
#define _SC_EXPRESSION_H_

#include <stdio.h>

typedef struct Expression Expression;
#include "variable.h"
#include "value.h"

struct Expression {
	Variable* var;
};


/* Constructor */
/*
 This method consumes the `var` argument, which is an unusual variable:
  - Its name is NULL unless there is an assignment
  - Its value is not simplified. It will store the entire expression tree
*/
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
char* Expression_verbose(Expression* expr, Context* ctx);
char* Expression_repr(Expression* expr, Context* ctx);
void Expression_fprint(FILE* fp, Expression* expr, Context* ctx, int verbosity);
void Expression_print(Expression* expr, Context* ctx, int verbosity);

#endif
