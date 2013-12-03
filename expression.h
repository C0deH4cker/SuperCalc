/*
  expression.h
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_EXPRESSION_H_
#define _SC_EXPRESSION_H_

#include <stdio.h>
#include <stdbool.h>

typedef struct Expression Expression;
#include "variable.h"
#include "value.h"
#include "supercalc.h"
#include "context.h"

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
Value* Expression_eval(Expression* expr, Context* ctx, VERBOSITY v);

/* Printing */
char* Expression_verbose(Expression* expr, Context* ctx);
char* Expression_repr(Expression* expr, Context* ctx, bool pretty);
void Expression_print(Expression* expr, SuperCalc* sc, VERBOSITY v);

#endif
