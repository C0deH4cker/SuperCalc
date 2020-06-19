/*
  value.h
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_VALUE_H
#define SC_VALUE_H

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
#include "function.h"
#include "builtin.h"
#include "placeholder.h"


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
	VAL_FUNC,
	VAL_BUILTIN,
	VAL_PLACE
} VALTYPE;


struct Value {
	VALTYPE type;
	union {
		              long long    ival;
		              double       rval;
		OWNED NONNULL Fraction*    frac;
		OWNED NONNULL Vector*      vec;
		OWNED NONNULL UnOp*        term;
		OWNED NONNULL BinOp*       expr;
		OWNED NONNULL FuncCall*    call;
		OWNED NONNULL Variable*    var;
		OWNED NONNULL Error*       err;
		OWNED NONNULL Function*    func;
		OWNED NONNULL Builtin*     blt;
		OWNED NONNULL Placeholder* ph;
	};
};

/* Default parser callback errors on '@' */
extern parser_cb default_cb;

/* Value constructors */
/* Each method which takes an object pointer as an argument consumes it */
OWNED NONNULL Value* ValEnd(void);
OWNED NONNULL Value* ValErr(OWNED NONNULL Error* err);
OWNED NONNULL Value* ValNeg(void);
OWNED NONNULL Value* ValInt(long long val);
OWNED NONNULL Value* ValReal(double val);
OWNED NONNULL Value* ValFrac(OWNED NONNULL Fraction* frac);
OWNED NONNULL Value* ValExpr(OWNED NONNULL BinOp* expr);
OWNED NONNULL Value* ValUnary(OWNED NONNULL UnOp* term);
OWNED NONNULL Value* ValCall(OWNED NONNULL FuncCall* call);
OWNED NONNULL Value* ValVar(OWNED NONNULL Variable* var);
OWNED NONNULL Value* ValVec(OWNED NONNULL Vector* vec);
OWNED NONNULL Value* ValFunc(OWNED NONNULL Function* func);
OWNED NONNULL Value* ValBuiltin(OWNED NONNULL Builtin* blt);
OWNED NONNULL Value* ValPlace(OWNED NONNULL Placeholder* ph);

/* Destructor */
void Value_free(OWNED NULLABLE Value* val);

/* Copying */
OWNED NULLABLE_WHEN(val == NULL) Value* Value_copy(NULLABLE const Value* val);

void Value_setScope(UNOWNED NONNULL Value* val, NULLABLE const Context* ctx);

/* Evaluation */
OWNED NONNULL Value* Value_eval(NONNULL const Value* val, NONNULL const Context* ctx);
bool Value_isCallable(NONNULL const Value* val);

/* Conversion */
double Value_asReal(NONNULL const Value* val);

/* Parsing */
OWNED NONNULL Value* Value_parseTop(INOUT NONNULL const char** expr);
OWNED NONNULL Value* Value_parse(INOUT NONNULL const char** expr, char sep, char end, NONNULL parser_cb* cb);
OWNED NONNULL Value* Value_next(INOUT NONNULL const char** expr, char sep, char end, NONNULL parser_cb* cb);

/* Printing */
OWNED NONNULL char* Value_repr(NONNULL const Value* val, bool pretty, bool top);
OWNED NONNULL char* Value_wrap(NONNULL const Value* val, bool top);
OWNED NONNULL char* Value_verbose(NONNULL const Value* val, unsigned indent);
OWNED NONNULL char* Value_xml(NONNULL const Value* val, unsigned indent);
void Value_print(NONNULL const Value* val, VERBOSITY v);

#endif /* SC_VALUE_H */
