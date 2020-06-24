/*
  tests.c
  sc_tests

  Created by C0deH4cker on 6/24/20.
  Copyright (c) 2020 C0deH4cker. All rights reserved.
*/

#include "utest/utest.h"
#include "test_helpers.h"
#include "value.h"


UTEST_MAIN();


struct SC {
	Context* _Nonnull ctx;
	Value* _Nullable parsed;
	Value* _Nullable evaled;
};

UTEST_F_SETUP(SC) {
	ASSERT_TRUE(true);
	F->ctx = Context_new();
	register_math(F->ctx);
	register_vector(F->ctx);
}

UTEST_F_TEARDOWN(SC) {
	ASSERT_TRUE(true);
	destroy(F->ctx);
	destroy(F->parsed);
	destroy(F->evaled);
}

UTEST_F(SC, number) {
	Value* val = PARSE("42");
	ASSERT_TRUE(IsValInt(val, 42));
	
	ASSERT_TRUE(IsValInt(EVAL(val), 42));
}

UTEST_F(SC, negativeNumber) {
	Value* val = PARSE("-2");
	ASSERT_TRUE(IsBinInts(val, BIN_MUL, -1, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(val), -2));
}

UTEST_F(SC, orderAdd) {
	Value* val = PARSE("4 + 3 + 2");
	ASSERT_TRUE(IsBinOp(val, BIN_ADD));
	ASSERT_TRUE(IsBinInts(val->expr->a, BIN_ADD, 4, 3));
	ASSERT_TRUE(IsValInt(val->expr->b, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(val), 9));
}

UTEST_F(SC, orderPow) {
	Value* val = PARSE("4 ^ 3 ^ 2");
	ASSERT_TRUE(IsBinOp(val, BIN_POW));
	ASSERT_TRUE(IsValInt(val->expr->a, 4));
	ASSERT_TRUE(IsBinInts(val->expr->b, BIN_POW, 3, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(val), 262144));
}

UTEST_F(SC, factorial) {
	Value* val = PARSE("5!");
	ASSERT_EQ(val->type, VAL_UNARY);
	ASSERT_EQ(val->term->type, UN_FACT);
	ASSERT_TRUE(IsValInt(val->term->a, 5));
	
	ASSERT_TRUE(IsValInt(EVAL(val), 120));
}
