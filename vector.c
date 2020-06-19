/*
  vector.c
  SuperCalc

  Created by Silas Schwarz on 11/16/13.
  Copyright (c) 2013 C0deH4cker and Silas Schwarz. All rights reserved.
*/

#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

#include "support.h"
#include "generic.h"
#include "error.h"
#include "arglist.h"
#include "value.h"
#include "context.h"
#include "binop.h"
#include "funccall.h"
#include "builtin.h"
#include "template.h"


static Value* vecScalarOp(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin);
static Value* vecMagOp(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin);
static Value* vecScalarOpRev(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin);
static Value* vecCompOp(const Vector* vector1, const Vector* vector2, const Context* ctx, BINTYPE bin);


Vector* Vector_new(ArgList* vals) {
	Vector* ret = fmalloc(sizeof(*ret));
	ret->vals = vals;
	return ret;
}

Vector* Vector_create(unsigned count, ...) {
	if(count < 1) {
		return NULL;
	}
	
	va_list ap;
	va_start(ap, count);
	
	Vector* ret = Vector_vcreate(count, ap);
	
	va_end(ap);
	return ret;
}

Vector* Vector_vcreate(unsigned count, va_list args) {
	return Vector_new(ArgList_vcreate(count, args));
}

void Vector_free(Vector* vec) {
	if(!vec) {
		return;
	}
	
	ArgList_free(vec->vals);
	free(vec);
}

Vector* Vector_copy(const Vector* vec) {
	if(!vec) {
		return NULL;
	}
	
	return Vector_new(ArgList_copy(vec->vals));
}

void Vector_setScope(Vector* vec, const Context* ctx) {
	ArgList_setScope(vec->vals, ctx);
}

Value* Vector_parse(const char** expr, parser_cb* cb) {
	Error* err = NULL;
	ArgList* vals = ArgList_parse(expr, ',', '>', cb, &err);
	
	if(vals == NULL) {
		return ValErr(err);
	}
	
	if(vals->count < 1) {
		ArgList_free(vals);
		return ValErr(syntaxError(*expr, "Vector must have at least 1 component."));
	}
	
	return ValVec(Vector_new(vals));
}

Value* Vector_eval(const Vector* vec, const Context* ctx) {
	ArgList* args = ArgList_eval(vec->vals, ctx);
	if(args == NULL) {
		return ValErr(ignoreError());
	}
	return ValVec(Vector_new(args));
}

static Value* vecScalarOp(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin) {
	ArgList* newv = ArgList_new(vec->vals->count);
	
	unsigned i;
	for(i = 0; i < vec->vals->count; i++) {
		/* Perform operation */
		BinOp* op = BinOp_new(bin, Value_copy(vec->vals->args[i]), Value_copy(scalar));
		Value* result = BinOp_eval(op, ctx);
		BinOp_free(op);
		
		/* Error checking */
		if(result->type == VAL_ERR) {
			ArgList_free(newv);
			return result;
		}
		
		/* Store result */
		newv->args[i] = result;
	}
	
	return ValVec(Vector_new(newv));
}

static Value* vecMagOp(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin) {
	/* Calculate old magnitude */
	Value* mag = Vector_magnitude(vec, ctx);
	
	/* Calculate new magnitude */
	BinOp* magOp = BinOp_new(bin, Value_copy(mag), Value_copy(scalar));
	Value* newMag = BinOp_eval(magOp, ctx);
	BinOp_free(magOp);
	
	/* Calculate scalar factor */
	BinOp* newDivOld = BinOp_new(BIN_DIV, newMag, mag);
	Value* scalFact = BinOp_eval(newDivOld, ctx);
	BinOp_free(newDivOld);
	/* Both newMag and mag are freed with newDivOld */
	
	/* Calculate new vector */
	return vecScalarOp(vec, scalFact, ctx, BIN_MUL);
}

static Value* vecScalarOpRev(const Vector* vec, const Value* scalar, const Context* ctx, BINTYPE bin) {
	ArgList* newv = ArgList_new(vec->vals->count);
	
	unsigned i;
	for(i = 0; i < vec->vals->count; i++) {
		/* Perform reverse operation */
		BinOp* op = BinOp_new(bin, Value_copy(scalar), Value_copy(vec->vals->args[i]));
		Value* result = BinOp_eval(op, ctx);
		BinOp_free(op);
		
		/* Error checking */
		if(result->type == VAL_ERR) {
			ArgList_free(newv);
			return result;
		}
		
		/* Store result */
		newv->args[i] = result;
	}
	
	return ValVec(Vector_new(newv));
}

static Value* vecCompOp(const Vector* vector1, const Vector* vector2, const Context* ctx, BINTYPE bin) {
	unsigned count = vector1->vals->count;
	if(count != vector2->vals->count && vector2->vals->count > 1) {
		return ValErr(mathError("Cannot %s vectors of different sizes.", binop_verb[bin]));
	}
	
	ArgList* newv = ArgList_new(count);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		/* Perform the specified operation on each matching component */
		Value* val2;
		if(vector2->vals->count == 1) {
			val2 = vector2->vals->args[0];
		}
		else {
			val2 = vector2->vals->args[i];
		}
		
		BinOp* op = BinOp_new(bin, Value_copy(vector1->vals->args[i]), Value_copy(val2));
		Value* result = BinOp_eval(op, ctx);
		BinOp_free(op);
		
		/* Error checking */
		if(result->type == VAL_ERR) {
			ArgList_free(newv);
			return result;
		}
		
		/* Store result */
		newv->args[i] = result;
	}
	
	return ValVec(Vector_new(newv));
}

Value* Vector_add(const Vector* vec, const Value* other, const Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_ADD);
	}
	
	return vecMagOp(vec, other, ctx, BIN_ADD);
}

Value* Vector_sub(const Vector* vec, const Value* other, const Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_SUB);
	}
	
	return vecMagOp(vec, other, ctx, BIN_SUB);
}

Value* Vector_rsub(const Vector* vec, const Value* scalar, const Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_SUB);
}

Value* Vector_mul(const Vector* vec, const Value* other, const Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_MUL);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_MUL);
}

Value* Vector_div(const Vector* vec, const Value* other, const Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_DIV);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_DIV);
}

Value* Vector_rdiv(const Vector* vec, const Value* scalar, const Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_DIV);
}

Value* Vector_pow(const Vector* vec, const Value* other, const Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_POW);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_POW);
}

Value* Vector_rpow(const Vector* vec, const Value* scalar, const Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_POW);
}

Value* Vector_dot(const Vector* vector1, const Vector* vector2, const Context* ctx) {
	unsigned count = vector1->vals->count;
	if(count != vector2->vals->count && vector2->vals->count != 1) {
		/* Both vectors must have the same number of values */
		return ValErr(mathError("Vectors must have the same dimensions for dot product."));
	}
	
	/* Store the total value of the dot product */
	Value* accum = ValInt(0);
	
	unsigned i;
	for(i = 0; i < count; i++) {
		Value* val2;
		if(vector2->vals->count == 1) {
			val2 = vector2->vals->args[0];
		}
		else {
			val2 = vector2->vals->args[i];
		}
		
		/* accum += v1[i] * val2 */
		TP(tp);
		accum = TP_EVAL(tp, ctx, "@@+@@*@@",
		                accum,
		                Value_copy(vector1->vals->args[i]),
		                Value_copy(val2));
	}
	
	return accum;
}

Value* Vector_cross(const Vector* u, const Vector* v, const Context* ctx) {
	/* Down to one statement from almost 100 lines because of TP_EVAL :) */
	/* Now up to two statements because MSVC doesn't support statement expressions :( */
	TP(tp);
	return TP_EVAL(tp, ctx,
		"<@2@*@6@ - @3@*@5@,"
		" @3@*@4@ - @1@*@6@,"
		" @1@*@5@ - @2@*@4@>",
		Value_copy(u->vals->args[0]), Value_copy(u->vals->args[1]), Value_copy(u->vals->args[2]),
		Value_copy(v->vals->args[0]), Value_copy(v->vals->args[1]), Value_copy(v->vals->args[2]));
}

Value* Vector_magnitude(const Vector* vec, const Context* ctx) {
	TP(tp);
	return TP_EVAL(tp, ctx, "sqrt(dot(@1v,@1v))", Vector_copy(vec));
}

Value* Vector_elem(const Vector* vec, const Value* index, const Context* ctx) {
	UNREFERENCED_PARAMETER(ctx);
	
	if(index->type != VAL_INT) {
		return ValErr(typeError("Subscript index must be an integer."));
	}
	
	if(index->ival < 0) {
		return ValErr(mathError("Subscript index cannot be negative."));
	}
	
	if(index->ival > UINT_MAX) {
		return ValErr(mathError("Subscript index %lld is too large.", index->ival));
	}
	
	unsigned idx = (unsigned)index->ival;
	
	if(idx >= vec->vals->count) {
		return ValErr(mathError("Index %u is out of range: [0-%u]", idx, vec->vals->count - 1));
	}
	
	return Value_copy(vec->vals->args[index->ival]);
}

char* Vector_repr(const Vector* vec, bool pretty) {
	char* ret;
	char* vals = ArgList_repr(vec->vals, pretty);
	
	asprintf(&ret, "<%s>", vals);
	
	free(vals);
	return ret;
}

char* Vector_wrap(const Vector* vec) {
	char* ret;
	char* vals = ArgList_wrap(vec->vals);
	
	asprintf(&ret, "<%s>", vals);
	
	free(vals);
	return ret;
}

char* Vector_verbose(const Vector* vec, unsigned indent) {
	char* ret;
	char* vals = ArgList_verbose(vec->vals, indent + 1);
	
	asprintf(&ret,
			 "Vector <\n"
				 "%2$s%3$s\n" /* vals */
			 "%1$s>",
			 indentation(indent), indentation(indent + 1),
			 vals);
	
	free(vals);
	return ret;
}

char* Vector_xml(const Vector* vec, unsigned indent) {
	/*
	 sc> ?x <pi, 7 - 3, 4!>
	 
	 <vec>
	   <var name="pi"/>
	   <sub>
	     <int>7</int>
	     <int>3</int>
	   </sub>
	   <fact>
	     <int>4</int>
	   </fact>
	 </vec>
	 
	 <3.14159265358979, 4, 24>
	*/
	char* ret;
	char* vals = ArgList_xml(vec->vals, indent + 1);
	
	asprintf(&ret,
			 "<vec>\n"
				 "%2$s\n" /* vals */
			 "%1$s</vec>",
			 indentation(indent),
			 vals);
	
	free(vals);
	return ret;
}

