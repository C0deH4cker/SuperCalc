/*
  value.h
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_VALUE_H_
#define _SC_VALUE_H_

#include <stdio.h>
#include <stdbool.h>

typedef struct Value Value;

/* Callback and associated data */
typedef struct parser_cb {
	Value* (*func)(const char** expr, void* data);
	void* data;
} parser_cb;

#include "fraction.h"
#include "unop.h"
#include "binop.h"
#include "funccall.h"
#include "variable.h"
#include "context.h"
#include "error.h"
#include "generic.h"
#include "vector.h"
#include "supercalc.h"


typedef enum {
	VAL_END = -3,
	VAL_ERR = -2,
	VAL_NEG = -1,
	VAL_INT = 0,
	VAL_REAL,
	VAL_FRAC,
	VAL_EXPR,
	VAL_UNARY,
	VAL_CALL,
	VAL_VAR,
	VAL_VEC,
	VAL_PLACE
} VALTYPE;


struct Value {
	VALTYPE type;
	union {
		long long  ival;
		double     rval;
		Fraction*  frac;
		Vector*    vec;
		UnOp*      term;
		BinOp*     expr;
		FuncCall*  call;
		char*      name;
		Error*     err;
		VALTYPE    fill;
	};
};

/* Does nothing */
parser_cb default_cb;

/* Value constructors */
/* Each method which takes an object pointer as an argument consumes it */
Value* ValEnd(void);
Value* ValErr(Error* err);
Value* ValNeg(void);
Value* ValInt(long long val);
Value* ValReal(double val);
Value* ValFrac(Fraction* frac);
Value* ValExpr(BinOp* expr);
Value* ValUnary(UnOp* term);
Value* ValCall(FuncCall* call);
Value* ValVar(const char* name);
Value* ValVec(Vector* vec);
Value* ValPlace(VALTYPE fill);

/* Destructor */
void Value_free(Value* val);

/* Copying */
Value* Value_copy(const Value* val);

/* Evaluation */
Value* Value_eval(const Value* expr, const Context* ctx);
Value* Value_coerce(const Value* val, const Context* ctx);

/* Conversion */
double Value_asReal(const Value* val);

/* Parsing */
Value* Value_parse(const char** expr, char sep, char end, parser_cb* cb);
Value* Value_next(const char** expr, char end, parser_cb* cb);

/* Printing */
char* Value_verbose(const Value* val, int indent);
char* Value_repr(const Value* val, bool pretty);
void Value_print(const Value* val, const SuperCalc* sc, VERBOSITY v);

#endif
