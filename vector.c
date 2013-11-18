/*
 vector.c
 SuperCalc
 
 Created by Silas Schwarz on 11/16/13.
 Copyright (c) 2013 C0deH4cker and Silas Schwarz. All rights reserved.
 */

#include "vector.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "generic.h"
#include "error.h"


Vector* Vector_new(ArgList* vals) {
	Vector* ret = fmalloc(sizeof(*ret));
	
	ret->vals = vals;
	
	return ret;
}

void Vector_free(Vector* vec) {
	ArgList_free(vec->vals);
	
	free(vec);
}

Vector* Vector_copy(Vector* vec) {
	return Vector_new(ArgList_copy(vec->vals));
}

Value* Vector_parse(const char** expr) {
	ArgList* vals = ArgList_parse(expr, ',', '>');
	
	if(vals->count < 1) {
		ArgList_free(vals);
		return ValErr(syntaxError("Vector must have at least 2 components."));
	}
	
	return ValVec(Vector_new(vals));
}

Value* Vector_eval(Vector* vec, Context* ctx) {
	return ValVec(Vector_new(ArgList_eval(vec->vals, ctx)));
}

static Value* vecScalarOp(Vector* vec, Value* scalar, Context *ctx, BINTYPE bin) {
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

static Value* vecMagOp(Vector* vec, Value* scalar, Context* ctx, BINTYPE bin) {
	/* Calculate old magnitude */
	Value* mag = Vector_magnitude(vec, ctx);
	
	/* Calculate new magnitude */
	BinOp* magOp = BinOp_new(bin, Value_copy(mag), Value_copy(scalar));
	Value* newMag = BinOp_eval(magOp, ctx);
	BinOp_free(magOp);
	
	/* Calculate scalar factor */
	BinOp* newDivOld = BinOp_new(BIN_DIV, newMag, mag);
	Value* scalFact = BinOp_eval(newDivOld, ctx);
	
	/* Calculate new vector */
	return vecScalarOp(vec, scalFact, ctx, BIN_MUL);
}

static Value* vecScalarOpRev(Vector* vec, Value* scalar, Context* ctx, BINTYPE bin) {
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

static Value* vecCompOp(Vector* vector1, Vector* vector2, Context* ctx, BINTYPE bin) {
	unsigned count = vector1->vals->count;
	if(count != vector2->vals->count && vector2->vals->count > 1) {
		return ValErr(mathError("Cannot %s vectors of different sizes.", binop_verb[bin]));
	}
	
	ArgList* newv = ArgList_new(count);
	for (long long i = 0; i < count; i++) {
		/* Perform the specified operation on each matching component */
		BinOp* op = BinOp_new(bin, Value_copy(vector1->vals->args[i]), Value_copy((vector2->vals->count == 1 ? vector2->vals->args[0] : vector2->vals->args[i])));
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

Value* Vector_add(Vector* vec, Value* other, Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_ADD);
	}
	return vecMagOp(vec, other, ctx, BIN_ADD);
}

Value* Vector_sub(Vector* vec, Value* other, Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_SUB);
	}
	
	return vecMagOp(vec, other, ctx, BIN_SUB);
}

Value* Vector_rsub(Vector* vec, Value* scalar, Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_SUB);
}

Value* Vector_mul(Vector* vec, Value* other, Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_MUL);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_MUL);
}

Value* Vector_div(Vector* vec, Value* other, Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_DIV);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_DIV);
}

Value* Vector_rdiv(Vector* vec, Value* scalar, Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_DIV);
}

Value* Vector_pow(Vector* vec, Value* other, Context* ctx) {
	if(other->type == VAL_VEC) {
		return vecCompOp(vec, other->vec, ctx, BIN_POW);
	}
	
	return vecScalarOp(vec, other, ctx, BIN_POW);
}

Value* Vector_rpow(Vector* vec, Value* scalar, Context* ctx) {
	return vecScalarOpRev(vec, scalar, ctx, BIN_POW);
}

static Value* Vector_dot(Value* vector1, Value* vector2, Context* ctx) {
	unsigned count = vector1->vec->vals->count;
	if(count != vector2->vec->vals->count && vector2->vec->vals->count != 1) {
		/* Both vectors must have the same number of values */
		return ValErr(mathError("Vectors must have the same dimensions for dot product."));
	}
	
	Value* total = ValInt(0); /* store the total value of the dot product */
	
	unsigned i;
	for(i = 0; i < count; i++) {
		/* Multiply v1[i] and v2[i] */
		BinOp* mul = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[i]), Value_copy((vector2->vec->vals->count == 1 ? vector2->vec->vals->args[0] : vector2->vec->vals->args[i])));
		
		Value* part = BinOp_eval(mul, ctx);
		BinOp_free(mul);
		
		/* Accumulate the sum for all products */
		BinOp* add = BinOp_new(BIN_ADD, part, total);
		total = BinOp_eval(add, ctx);
		BinOp_free(add);
	}
	
	Value_free(vector1);
	Value_free(vector2);
	
	return total;
}

Value* Vector_magnitude(Vector* vec, Context* ctx) {
	/* |v|^2 = dot(v,v) */
	Vector* value1 = Vector_copy(vec);
	Vector* value2 = Vector_copy(vec);
	Value* v1 = ValVec(value1);
	Value* v2 = ValVec(value2);
	Value* magSqu = Vector_dot(v1, v2, ctx);
	
	/* Calculate actual magnitude */
	ArgList* args = ArgList_new(1);
	args->args[0] = magSqu;
	FuncCall *sqr = FuncCall_new("sqrt", args);
	Value* mag = FuncCall_eval(sqr, ctx);
	FuncCall_free(sqr);
	return mag;
}

static Value* eval_dot(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		/* Two vectors are required for a dot product */
		return ValErr(builtinArgs("dot", 2, arglist->count));
	}
	
	Value* vector1 = Value_eval(arglist->args[0], ctx);
	if(vector1->type == VAL_ERR) {
		return vector1;
	}
	
	Value* vector2 = Value_eval(arglist->args[1], ctx);
	if(vector2->type == VAL_ERR) {
		Value_free(vector1);
		return vector2;
	}
	
	if(vector1->type != VAL_VEC || vector2->type != VAL_VEC) {
		/* Both values must be vectors */
		return ValErr(typeError("Builtin dot expects two vectors."));
	}
	
	return Vector_dot(vector1, vector2, ctx);
}

static Value* eval_cross(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		/* Two vectors are required for a cross product */
		return ValErr(builtinArgs("cross", 2, arglist->count));
	}
	
	Value* vector1 = Value_eval(arglist->args[0], ctx);
	if(vector1->type == VAL_ERR) {
		return vector1;
	}
	
	Value* vector2 = Value_eval(arglist->args[1], ctx);
	if(vector2->type == VAL_ERR) {
		Value_free(vector1);
		return vector2;
	}
	
	if(vector1->type != VAL_VEC || vector2->type != VAL_VEC) {
		/* Both values must be vectors */
		return ValErr(typeError("Builtin dot expects two vectors."));
	}
	
	if(vector1->vec->vals->count != 3) {
		/* Vectors must each have a size of 3 */
		return ValErr(mathError("Vectors must each have a size of 3 for cross product."));
	}
	
	/* First cross multiplication */
	BinOp* i_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[1]), Value_copy(vector2->vec->vals->args[2]));
	BinOp* i_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[2]), Value_copy(vector2->vec->vals->args[1]));
	
	/* Evaluate multiplications */
	Value* i_pos = BinOp_eval(i_pos_op, ctx);
	Value* i_neg = BinOp_eval(i_neg_op, ctx);
	
	BinOp_free(i_pos_op);
	BinOp_free(i_neg_op);
	
	/* Error checking */
	if(i_pos->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(i_neg);
		return i_pos;
	}
	if(i_neg->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(i_pos);
		return i_neg;
	}
	
	/* Subtract products */
	BinOp* i_op = BinOp_new(BIN_SUB, i_pos, i_neg);
	Value* i_val = BinOp_eval(i_op, ctx);
	BinOp_free(i_op);
	
	if(i_val->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		return i_val;
	}
	
	/* Part 2 */
	BinOp* j_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[0]), Value_copy(vector2->vec->vals->args[2]));
	BinOp* j_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[2]), Value_copy(vector2->vec->vals->args[0]));
	
	Value* j_pos = BinOp_eval(j_pos_op, ctx);
	Value* j_neg = BinOp_eval(j_neg_op, ctx);
	
	BinOp_free(j_pos_op);
	BinOp_free(j_neg_op);
	
	if(j_pos->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(j_neg);
		return j_pos;
	}
	if(j_neg->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(j_pos);
		return j_neg;
	}
	
	BinOp* j_op = BinOp_new(BIN_SUB, j_pos, j_neg);
	Value* j_val = BinOp_eval(j_op, ctx);
	BinOp_free(j_op);
	
	if(j_val->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		return j_val;
	}
	
	/* Part 3 */
	BinOp* k_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[0]), Value_copy(vector2->vec->vals->args[1]));
	BinOp* k_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[1]), Value_copy(vector2->vec->vals->args[0]));
	
	Value* k_pos = BinOp_eval(k_pos_op, ctx);
	Value* k_neg = BinOp_eval(k_neg_op, ctx);
	
	BinOp_free(k_pos_op);
	BinOp_free(k_neg_op);
	
	if(k_pos->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(k_neg);
		return k_pos;
	}
	if(k_neg->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		Value_free(k_pos);
		return k_neg;
	}
	
	BinOp* k_op = BinOp_new(BIN_SUB, k_pos, k_neg);
	Value* k_val = BinOp_eval(k_op, ctx);
	BinOp_free(k_op);
	
	if(k_val->type == VAL_ERR) {
		Value_free(vector1);
		Value_free(vector2);
		return k_val;
	}
	
	ArgList* args = ArgList_create(3, i_val, j_val, k_val);
	return ValVec(Vector_new(args));
}

static Value* eval_map(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		return ValErr(builtinArgs("map", 2, arglist->count));
	}
	
	Value* func = Value_copy(arglist->args[0]);
	if(func->type != VAL_VAR) {
		Value* val = Value_eval(func, ctx);
		Value_free(func);
		
		if(val->type == VAL_ERR)
			return val;
		
		if(val->type != VAL_VAR) {
			Value_free(val);
			return ValErr(typeError("Builtin 'map' expects a callable as its first argument."));
		}
		
		func = val;
	}
	
	Value* vec = Value_eval(arglist->args[1], ctx);
	if(vec->type == VAL_ERR) {
		Value_free(func);
		return vec;
	}
	
	if(func->type != VAL_VAR) {
		Value_free(func);
		Value_free(vec);
		return ValErr(typeError("Builtin 'map' expects a callable as its first argument."));
	}
	
	if(vec->type != VAL_VEC) {
		Value_free(func);
		Value_free(vec);
		return ValErr(typeError("Builtin 'map' expects a vector as its second argument."));
	}
	
	ArgList* mapping = ArgList_new(vec->vec->vals->count);
	
	/* Don't evaluate the call now. Let Builtin_eval do this for us */
	unsigned i;
	for(i = 0; i < mapping->count; i++) {
		ArgList* arg = ArgList_create(1, Value_copy(vec->vec->vals->args[i]));
		Value* call = ValCall(FuncCall_new(func->name, arg));
		
		mapping->args[i] = call;
	}
	
	Value_free(func);
	Value_free(vec);
	
	return ValVec(Vector_new(mapping));
}

static Value* eval_vval(Context* ctx, ArgList* arglist) {
	if (arglist->count != 2) {
		return ValErr(builtinArgs("vval", 2, arglist->count));
	}
	/* Get evaluated values */
	Value *vector = Value_eval(arglist->args[0], ctx);
	Value *index = Value_eval(arglist->args[1], ctx);
	
	/* Check vector type */
	if (vector->type != VAL_VEC) {
		return ValErr(typeError("Unexpected type: %d", vector->type));
	}
	
	/* Get actual value */
	Value *ret = Vector_value(vector->vec, index, ctx);
	
	/* Free allocated memory */
	Value_free(vector);
	Value_free(index);
	return ret;
}

static const char* vector_names[] = {
	"dot", "cross", "map", "vval"
};
static builtin_eval_t vector_funcs[] = {
	&eval_dot, &eval_cross, &eval_map, &eval_vval
};

/* This is just a copy of register_math remade for vectors */
void Vector_register(Context *ctx) {
	unsigned count = sizeof(vector_names) / sizeof(vector_names[0]);
	unsigned i;
	for(i = 0; i < count; i++) {
		Builtin* blt = Builtin_new(vector_names[i], vector_funcs[i]);
		Builtin_register(blt, ctx);
	}
}

Value* Vector_value(Vector* vec, Value* index, Context* ctx) {
	if (index->type != VAL_INT) {
		return ValErr(typeError("Unexpected type: %d", index->type));
	}
	if (index->ival >= vec->vals->count) {
		return ValErr(mathError("Index %d is out of range: 0-%d", index->ival, vec->vals->count));
	}
	return Value_copy(vec->vals->args[index->ival]);
}

char* Vector_verbose(Vector* vec, int indent) {
	char* ret;
	
	char* current = spaces(indent);
	char* vals = ArgList_verbose(vec->vals, indent + IWIDTH);
	
	asprintf(&ret, "Vector <\n%s%s>",
			 vals,
			 current);
	
	free(current);
	free(vals);
	
	return ret;
}

char* Vector_repr(Vector* vec) {
	char* ret;
	
	char* vals = ArgList_repr(vec->vals);
	asprintf(&ret, "<%s>", vals);
	free(vals);
	
	return ret;
}

