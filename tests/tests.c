/*
  tests.c
  sc_tests

  Created by C0deH4cker on 6/24/20.
  Copyright (c) 2020 C0deH4cker. All rights reserved.
*/

#include <stddef.h>
#include <stdbool.h>
#include <math.h>

#include "utest/utest.h"
#include "test_helpers.h"
#include "value.h"


UTEST_MAIN();


UTEST_F_SETUP(SC) {
	ASSERT_TRUE(true);
	F->ctx = Context_new();
	register_math(F->ctx);
	register_vector(F->ctx);
	g_inputFile = fopen("/dev/null", "r");
	g_inputFileName = NULL;
}

UTEST_F_TEARDOWN(SC) {
	ASSERT_TRUE(true);
	_fixtureNext(F);
	Context_free(F->ctx);
	F->ctx = NULL;
	fclose(g_inputFile);
	g_inputFile = NULL;
}


UTEST_F(SC, number) {
	Value* val = PARSEVAL("42");
	ASSERT_TRUE(IsValInt(val, 42));
	
	ASSERT_TRUE(IsValInt(EVAL(), 42));
}

UTEST_F(SC, ansDefault) {
	ASSERT_TRUE(IsValInt(EVALSTR("ans"), 0));
}

UTEST_F(SC, negativeNumber) {
	Value* val = PARSEVAL("-2");
	ASSERT_TRUE(IsBinInts(val, BIN_MUL, -1, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(), -2));
}

UTEST_F(SC, implicitMul) {
	Value* val = PARSEVAL("2pi");
	ASSERT_TRUE(IsBinOp(val, BIN_MUL));
	ASSERT_TRUE(IsValInt(val->expr->a, 2));
	ASSERT_EQ(val->expr->b->type, VAL_VAR);
	ASSERT_STREQ(val->expr->b->name, "pi");
	
	ASSERT_TRUE(IsValReal(EVAL(), 2.0 * M_PI));
}

UTEST_F(SC, orderAdd) {
	Value* val = PARSEVAL("4 + 3 + 2");
	ASSERT_TRUE(IsBinOp(val, BIN_ADD));
	ASSERT_TRUE(IsBinInts(val->expr->a, BIN_ADD, 4, 3));
	ASSERT_TRUE(IsValInt(val->expr->b, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(), 9));
}

UTEST_F(SC, orderPow) {
	Value* val = PARSEVAL("4 ^ 3 ^ 2");
	ASSERT_TRUE(IsBinOp(val, BIN_POW));
	ASSERT_TRUE(IsValInt(val->expr->a, 4));
	ASSERT_TRUE(IsBinInts(val->expr->b, BIN_POW, 3, 2));
	
	ASSERT_TRUE(IsValInt(EVAL(), 262144));
}

UTEST_F(SC, factorial) {
	Value* val = PARSEVAL("5!");
	ASSERT_EQ(val->type, VAL_UNARY);
	ASSERT_EQ(val->term->type, UN_FACT);
	ASSERT_TRUE(IsValInt(val->term->a, 5));
	
	ASSERT_TRUE(IsValInt(EVAL(), 120));
}

UTEST_F(SC, divFrac) {
	ASSERT_TRUE(IsBinInts(PARSEVAL("3 / 4"), BIN_DIV, 3, 4));
	ASSERT_TRUE(IsValFrac(EVAL(), 3, 4));
}

UTEST_F(SC, fracPow) {
	Value* res = EVALSTR("(2 / 7) ^ 2");
	ASSERT_TRUE(IsValFrac(res, 4, 49));
}

UTEST_F(SC, factFracPow) {
	Value* res = EVALSTR("-(3 + 4!/7)^3");
	ASSERT_TRUE(IsValFrac(res, -91125, 343));
}

UTEST_F(SC, sqrtFrac) {
	ASSERT_TRUE(IsValFrac(EVALSTR("sqrt(9/16)"), 3, 4));
}

UTEST_F(SC, fracToInt) {
	ASSERT_TRUE(IsValInt(EVALSTR("sqrt(9/16) + 5/4"), 2));
}

UTEST_F(SC, intPowZero) {
	ASSERT_TRUE(IsValInt(EVALSTR("3^0"), 1));
}

UTEST_F(SC, realPowZero) {
	ASSERT_TRUE(IsValInt(EVALSTR("3.0^0"), 1));
}

UTEST_F(SC, fracPowZero) {
	ASSERT_TRUE(IsValInt(EVALSTR("(1/2)^0"), 1));
}

UTEST_F(SC, vecPowZero) {
	ASSERT_TRUE(IsValVecInts(EVALSTR("<1, 2, 3>^0"), 3, 1,1,1));
}

UTEST_F(SC, varAssignment) {
	Statement* stmt;
	Value* val;
	Value* res;
	
	stmt = PARSE("a = 5");
	ASSERT_NE(stmt->var->name, NULL);
	ASSERT_STREQ(stmt->var->name, "a");
	ASSERT_TRUE(IsValInt(stmt->var->val, 5));
	EVALNOERR();
	
	val = PARSEVAL("a * 3");
	ASSERT_TRUE(IsBinOp(val, BIN_MUL));
	ASSERT_EQ(val->expr->a->type, VAL_VAR);
	ASSERT_STREQ(val->expr->a->name, "a");
	ASSERT_TRUE(IsValInt(val->expr->b, 3));
	
	res = EVAL();
	ASSERT_TRUE(IsValInt(res, 15));
	
	res = EVALSTR("b = 2 * a + 4");
	ASSERT_TRUE(IsValInt(res, 14));
	
	res = EVALSTR("ans + 4");
	ASSERT_TRUE(IsValInt(res, 18));
}

UTEST_F(SC, inPlaceAssignment) {
	ASSERT_TRUE(IsValInt(EVALSTR("x = 4"), 4));
	ASSERT_TRUE(IsValInt(EVALSTR("x += 3"), 7));
	ASSERT_TRUE(IsValInt(EVALSTR("x *= 2"), 14));
	ASSERT_TRUE(IsValFrac(EVALSTR("x /= 3"), 14, 3));
	ASSERT_TRUE(IsValInt(EVALSTR("x *= 6"), 28));
	ASSERT_TRUE(IsValInt(EVALSTR("x %= 2"), 0));
	ASSERT_TRUE(IsValInt(EVALSTR("x += 8"), 8));
	ASSERT_TRUE(IsValInt(EVALSTR("x ^= 2"), 64));
}

UTEST_F(SC, simpleFunction) {
	Statement* stmt = PARSE("f(x) = 3x + 4");
	ASSERT_NE(stmt->var->name, NULL);
	ASSERT_EQ(strcmp(stmt->var->name, "f"), 0);
	
	Value* res = EVAL();
	ASSERT_EQ(res->type, VAL_FUNC);
	ASSERT_EQ(res->func->argcount, 1u);
	ASSERT_EQ(strcmp(res->func->argnames[0], "x"), 0);
	
	ASSERT_TRUE(IsBinOp(res->func->body, BIN_ADD));
	ASSERT_TRUE(IsBinOp(res->func->body->expr->a, BIN_MUL));
	
	ASSERT_TRUE(IsValInt(EVALSTR("f(7)"), 25));
}

UTEST_F(SC, multipleArguments) {
	Value* res = EVALSTR("f(x, y) = x + y");
	ASSERT_EQ(res->type, VAL_FUNC);
	ASSERT_EQ(res->func->argcount, 2u);
	
	ASSERT_TRUE(IsValInt(EVALSTR("f(3, 5)"), 8));
	
	RUN("g(x, y, z) = f(x, y) * z");
	ASSERT_TRUE(IsValInt(EVALSTR("g(1, 2, 3)"), 9));
}

UTEST_F(SC, variableShadowing) {
	RUN("x = 7");
	RUN("f(x) = 4x");
	ASSERT_TRUE(IsValInt(EVALSTR("f(6)"), 24));
	ASSERT_TRUE(IsValInt(EVALSTR("x"), 7));
}

UTEST_F(SC, funcWithGlobals) {
	RUN("myFunc(arg) = 3 * arg + glb");
	RUN("glb = 7");
	ASSERT_TRUE(IsValInt(EVALSTR("myFunc(4)"), 19));
	RUN("glb += 3");
	ASSERT_TRUE(IsValInt(EVALSTR("myFunc(4)"), 22));
}

UTEST_F(SC, funcWithAns) {
	RUN("other(x) = x + ans");
	RUN("8")
	ASSERT_TRUE(IsValInt(EVALSTR("other(4)"), 12));
	ASSERT_TRUE(IsValInt(EVALSTR("other(4)"), 16));
}

UTEST_F(SC, assignFunc) {
	RUN("f(x, y) = x^2 - y^2");
	RUN("g = f");
	ASSERT_TRUE(IsValInt(EVALSTR("g(4, 3)"), 7));
}

UTEST_F(SC, getFunc) {
	RUN("h(x) = x^2");
	RUN("getFunc() = h");
	RUN("func = getFunc()");
	ASSERT_TRUE(IsValInt(EVALSTR("func(3)"), 9));
	
	ASSERT_TRUE(IsValInt(EVALSTR("getFunc()(4)"), 16));
}

UTEST_F(SC, vectorParsing) {
	Value* val = PARSEVAL("<1, 2>");
	ASSERT_EQ(val->type, VAL_VEC);
	ASSERT_EQ(val->vec->vals->count, 2u);
	ASSERT_TRUE(IsValInt(val->vec->vals->args[0], 1));
	ASSERT_TRUE(IsValInt(val->vec->vals->args[1], 2));
}

UTEST_F(SC, vectorArithmetic) {
	RUN("a = <1, 2, 3>");
	RUN("b = <6, 5, 3>");
	ASSERT_TRUE(IsValVecInts(EVALSTR("a - b"), /*count=*/3, -5,-3,0));
	ASSERT_TRUE(IsValVecInts(EVALSTR("6 * a - b"), /*count=*/3, 0,7,15));
}

UTEST_F(SC, vectorCrossDot) {
	RUN("a = <1, 2, 3>");
	RUN("b = <6, 5, 3>");
	ASSERT_TRUE(IsValVecInts(EVALSTR("cross(a, b)"), 3, -9,15,-7));
	ASSERT_TRUE(IsValInt(EVALSTR("dot(a, b)"), 25));
}

UTEST_F(SC, nestedVectors) {
	RUN("a = <1, 2, 3, <4, 5>>");
	ASSERT_TRUE(IsValInt(EVALSTR("a[1]"), 2));
	ASSERT_VALEQ(EVALSTR("a[3]"), VAL_VEC, 2,
		VAL_INT, 4ll,
		VAL_INT, 5ll
	);
}

UTEST_F(SC, getFuncVecChained) {
	RUN("getVec() = <1, 2, 3, <4, 5>>");
	ASSERT_TRUE(IsValInt(EVALSTR("getVec()[3][1]"), 5));
	RUN("getFunc() = getVec");
	ASSERT_TRUE(IsValInt(EVALSTR("getFunc()()[3][0]"), 4));
}

UTEST_F(SC, vectorMap) {
	RUN("a = <3, 4, 5>");
	RUN("f(x) = 2x + 1");
	ASSERT_TRUE(IsValVecInts(EVALSTR("map(f, a)"), 3, 7,9,11));
	
	/*
	 * <0, 1, 0, -1, 0>
	 * Can't directly compare these doubles against 0.0, -1.0, or 1.0,
	 * as there's some small error margin and the values aren't perfect.
	 */
	RUN("angles = <0, pi/2, pi, 3pi/2, 2pi>");
	ASSERT_VALEQ(EVALSTR("map(sin, angles)"), VAL_VEC, 5,
		VAL_APPROX, 0.0, 1e-13,
		VAL_APPROX, 1.0, 1e-13,
		VAL_APPROX, 0.0, 1e-13,
		VAL_APPROX, -1.0, 1e-13,
		VAL_APPROX, 0.0, 1e-13
	);
}

UTEST_F(SC, vectorMul) {
	ASSERT_TRUE(IsValVecInts(EVALSTR("<1, 2, 3> * <4, 7, 2>"), 3, 4,14,6));
}

UTEST_F(SC, vectorDiv) {
	/* <1/4, 2/3, 5/2> */
	ASSERT_VALEQ(EVALSTR("<1, 4, 5> / <4, 6, 2>"), VAL_VEC, 3,
		VAL_FRAC, 1ll,4ll,
		VAL_FRAC, 2ll,3ll,
		VAL_FRAC, 5ll,2ll
	);
}

UTEST_F(SC, vectorAns) {
	RUN("a = <4, 7, -3>");
	
	/* <4.92998110995055, 8.62746694241347, -3.69748583246292> */
	ASSERT_VALEQ(EVALSTR("a + 2"), VAL_VEC, 3,
		VAL_APPROX, 4.92998110995055, 1e-13,
		VAL_APPROX, 8.62746694241347, 1e-13,
		VAL_APPROX, -3.69748583246292, 1e-13
	);
	
	/* <1.64332703665018, 2.87582231413782, -1.23249527748764> */
	ASSERT_VALEQ(EVALSTR("ans / 3"), VAL_VEC, 3,
		VAL_APPROX, 1.64332703665018, 1e-13,
		VAL_APPROX, 2.87582231413782, 1e-13,
		VAL_APPROX, -1.23249527748764, 1e-13
	);
	
	/* <2.70052374938548, 8.27035398249302, 1.51904460902933> */
	ASSERT_VALEQ(EVALSTR("ans ^ 2"), VAL_VEC, 3,
		VAL_APPROX, 2.70052374938548, 1e-13,
		VAL_APPROX, 8.27035398249302, 1e-13,
		VAL_APPROX, 1.51904460902933, 1e-13
	);
	
	/* <0.740597078790777, 0.241827617564335, 1.31661702896138> */
	ASSERT_VALEQ(EVALSTR("2 / ans"), VAL_VEC, 3,
		VAL_APPROX, 0.740597078790777, 1e-13,
		VAL_APPROX, 0.241827617564335, 1e-13,
		VAL_APPROX, 1.31661702896138, 1e-13
	);
}

UTEST_F(SC, missingVars) {
	ASSERT_VALEQ(EVALSTR("a"), VAL_ERR,
		ERR_NAME, "Name Error: No variable named 'a' found.\n"
	);
	
	ASSERT_VALEQ(EVALSTR("f(5)"), VAL_ERR,
		ERR_NAME, "Name Error: No variable named 'f' found.\n"
	);
}

UTEST_F(SC, errDivByZero) {
	ASSERT_VALEQ(EVALSTR("3 / (1 - 1)"), VAL_ERR,
		ERR_MATH, "Math Error: Division by zero.\n"
	);
	
	ASSERT_VALEQ(EVALSTR("3 / 0.0"), VAL_ERR,
		ERR_MATH, "Math Error: Division by zero.\n"
	);
}

UTEST_F(SC, errSqrtArgs) {
	ASSERT_VALEQ(EVALSTR("sqrt()"), VAL_ERR,
		ERR_TYPE, "Type Error: Builtin 'sqrt' expects 1 argument, not 0.\n"
	);
	
	ASSERT_VALEQ(EVALSTR("sqrt(4, 3)"), VAL_ERR,
		ERR_TYPE, "Type Error: Builtin 'sqrt' expects 1 argument, not 2.\n"
	);
}

UTEST_F(SC, sqrtComplex) {
	ASSERT_VALEQ(EVALSTR("sqrt(-1)"), VAL_ERR,
		ERR_MATH, "Math Error: Power result is complex\n"
	);
}

UTEST_F(SC, errBadChar) {
	ASSERT_VALEQ(PARSEVAL("17 $ 8"), VAL_ERR,
		ERR_SYNTAX, "Syntax Error: Unexpected character: '$'.\n"
	);
}

UTEST_F(SC, errEarlyEnd) {
	ASSERT_VALEQ(PARSEVAL("(3 + )"), VAL_ERR,
		ERR_SYNTAX, "Syntax Error: Premature end of input.\n"
	);
}

UTEST_F(SC, errCallPi) {
	ASSERT_VALEQ(EVALSTR("pi(1, 2)"), VAL_ERR,
		ERR_TYPE, "Type Error: Builtin 'pi' is not a function.\n"
	);
}

UTEST_F(SC, vecMapSqrtFracs) {
	RUN("add1(x) = 1 + x");
	
	/* <2, 3, 4, 5, 5.47213595499958, 7/3> */
	ASSERT_VALEQ(
		EVALSTR("map(add1, map(sqrt, <1, 4, 9, 16, 20, 16/9>))"),
		VAL_VEC, 6,
			VAL_INT, 2ll,
			VAL_INT, 3ll,
			VAL_INT, 4ll,
			VAL_INT, 5ll,
			VAL_APPROX, 5.47213595499958, 1e-13,
			VAL_FRAC, 7ll, 3ll
	);
}
