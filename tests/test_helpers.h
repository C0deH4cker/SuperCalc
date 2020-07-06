/*
  test_helpers.h
  SuperCalc

  Created by C0deH4cker on 6/24/20.
  Copyright (c) 2020 C0deH4cker. All rights reserved.
*/

#ifndef SC_TEST_HELPERS_H
#define SC_TEST_HELPERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#include "annotations.h"
#include "value.h"
#include "context.h"
#include "defaults.h"
#include "statement.h"
#include "template.h"


#define F utest_fixture

struct SC {
	Context* _Nonnull ctx;
	Statement* _Nullable stmt;
	Value* _Nullable evaled;
};


ASSUME_NONNULL_BEGIN


#define PARSE(str) (Statement* _Nonnull)(F->stmt = _parseStr(F, str))
#define PARSEVAL(str) (Value* _Nonnull)(PARSE(str)->var->val)
#define EVAL() (Value* _Nonnull)(F->evaled = Statement_eval(CAST_NONNULL(F->stmt), F->ctx, V_NONE))
#define EVALNOERR() ASSERT_NE(VAL_ERR, (VALTYPE)(EVAL()->type))
#define EVALSTR(str) ((void)PARSE(str), EVAL())
#define RUN(str) ASSERT_TRUE(EVALSTR(str)->type != VAL_ERR)

#define ASSERT_VALEQ(val, ...) ASSERT_TRUE(IsVal((val), __VA_ARGS__))

static inline void _fixtureNext(struct SC* fxt) {
	Statement_free(fxt->stmt);
	fxt->stmt = NULL;
	Value_free(fxt->evaled);
	fxt->evaled = NULL;
}

static inline Statement* _parseStr(struct SC* fxt, const char* str) {
	_fixtureNext(fxt);
	return Statement_parse(&str);
}

bool IsVal(const Value* _Nullable val, ...);
bool vIsVal(const Value* _Nullable val, va_list ap);

static inline bool IsValInt(const Value* _Nullable val, long long ival) {
	return IsVal(val, VAL_INT, ival);
}

static inline bool IsValReal(const Value* _Nullable val, double rval) {
	return IsVal(val, VAL_REAL, rval);
}

static inline bool IsValVecInts(const Value* _Nullable val, unsigned count, ...) {
	if(val == NULL || val->type != VAL_VEC || val->vec->vals->count != count) {
		return false;
	}
	
	bool same = true;
	
	va_list ap;
	va_start(ap, count);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		int v_i = va_arg(ap, int);
		if(!IsValInt(val->vec->vals->args[i], (long long)v_i)) {
			same = false;
			break;
		}
	}
	
	va_end(ap);
	
	return same;
}

static inline bool IsBinOp(const Value* _Nullable val, BINTYPE type) {
	return val != NULL
		&& val->type == VAL_EXPR
		&& val->expr->type == type;
}

static inline bool IsBinInts(const Value* _Nullable val, BINTYPE type, long long a, long long b) {
	return IsBinOp(val, type)
		&& IsValInt(val->expr->a, a)
		&& IsValInt(val->expr->b, b);
}

static inline bool IsValFrac(const Value* _Nullable val, long long n, long long d) {
	return val->type == VAL_FRAC
		&& val->frac->n == n
		&& val->frac->d == d;
}

ASSUME_NONNULL_END

#endif /* SC_TEST_HELPERS_H */
