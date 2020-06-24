/*
  test_helpers.h
  SuperCalc

  Created by C0deH4cker on 6/24/20.
  Copyright (c) 2020 C0deH4cker. All rights reserved.
*/

#ifndef SC_TEST_HELPERS_H
#define SC_TEST_HELPERS_H

#include <stdbool.h>
#include "annotations.h"
#include "value.h"
#include "context.h"
#include "defaults.h"


#define F utest_fixture


ASSUME_NONNULL_BEGIN


#define PARSE(str) (Value* _Nonnull)(F->parsed = _parseStr(str))
#define EVAL(val) (Value* _Nonnull)(F->evaled = Value_eval((val), F->ctx))
#define EVALSTR(str) EVAL(PARSE(str))


static inline Value* _parseStr(const char* str) {
	return Value_parseTop(&str);
}

static inline bool IsValInt(const Value* _Nullable val, long long ival) {
	return val != NULL
		&& val->type == VAL_INT
		&& val->ival == ival;
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

ASSUME_NONNULL_END

#endif /* SC_TEST_HELPERS_H */
