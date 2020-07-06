/*
  value.h
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_VALUE_H
#define SC_VALUE_H

#include <stdbool.h>

#include "annotations.h"

typedef struct Value Value;

/* Callback and associated data */
typedef struct parser_cb {
	Value* _Nonnull (*_Nonnull func)(istring expr, void* _Nullable data);
	void* _Nullable data;
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


ASSUME_NONNULL_BEGIN

typedef enum {
	VAL_APPROX = -4,
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
		OWNED Fraction*    frac;
		OWNED Vector*      vec;
		OWNED UnOp*        term;
		OWNED BinOp*       expr;
		OWNED FuncCall*    call;
		OWNED char*        name;
		OWNED Error*       err;
		OWNED Function*    func;
		OWNED Builtin*     blt;
		OWNED Placeholder* ph;
	};
};


DECL(Value);

/* Default parser callback errors on '@' */
extern parser_cb default_cb;

/* Value constructors */
RETURNS_OWNED Value* ValEnd(void);
RETURNS_OWNED Value* ValErr(CONSUMED Error* err);
RETURNS_OWNED Value* ValNeg(void);
RETURNS_OWNED Value* ValInt(long long val);
RETURNS_OWNED Value* ValReal(double val);
RETURNS_OWNED Value* ValFrac(CONSUMED Fraction* frac);
RETURNS_OWNED Value* ValExpr(CONSUMED BinOp* expr);
RETURNS_OWNED Value* ValUnary(CONSUMED UnOp* term);
RETURNS_OWNED Value* ValCall(CONSUMED FuncCall* call);
RETURNS_OWNED Value* ValVar(CONSUMED char* name);
RETURNS_OWNED Value* ValVec(CONSUMED Vector* vec);
RETURNS_OWNED Value* ValFunc(CONSUMED Function* func);
RETURNS_OWNED Value* ValBuiltin(CONSUMED Builtin* blt);
RETURNS_OWNED Value* ValPlace(CONSUMED Placeholder* ph);

/* Destructor */
void Value_free(CONSUMED Value* _Nullable val);

/* Copying */
RETURNS_OWNED Value* Value_copy(const Value* val);

/* Evaluation */
RETURNS_OWNED Value* Value_eval(const Value* val, const Context* ctx);
RETURNS_OWNED Value* Value_coerce(const Value* val, const Context* ctx);
bool Value_isCallable(const Value* val);

/* Conversion */
double Value_asReal(const Value* val);

/* Parsing */
RETURNS_OWNED Value* Value_parseTop(INOUT istring expr);
RETURNS_OWNED Value* Value_parse(INOUT istring expr, char sep, char end, parser_cb* cb);
RETURNS_OWNED Value* Value_next(INOUT istring expr, char sep, char end, parser_cb* cb);

/* Printing */
RETURNS_OWNED char* Value_repr(const Value* val, bool pretty, bool top);
RETURNS_OWNED char* Value_wrap(const Value* val, bool top);
RETURNS_OWNED char* Value_verbose(const Value* val, unsigned indent);
RETURNS_OWNED char* Value_xml(const Value* val, unsigned indent);
void Value_print(const Value* val, VERBOSITY v);

METHOD_debugString(Value);

ASSUME_NONNULL_END

#endif /* SC_VALUE_H */
