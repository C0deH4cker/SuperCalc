/*
  defaults_vector.c
  SuperCalc

  Created by C0deH4cker on 1/15/14.
  Copyright (c) 2014 C0deH4cker. All rights reserved.
*/

#include "vector.h"
#include <stdbool.h>

#include "error.h"
#include "value.h"
#include "context.h"
#include "arglist.h"
#include "binop.h"
#include "builtin.h"

static Value* eval_dot(Context* ctx, ArgList* arglist, bool internal) {
	Value* ret;
	
	if(arglist->count != 2) {
		/* Two vectors are required for a dot product */
		return ValErr(builtinArgs("dot", 2, arglist->count));
	}
	
	Value* vector1 = Value_coerce(arglist->args[0], ctx);
	if(vector1->type == VAL_ERR) {
		return vector1;
	}
	
	Value* vector2 = Value_coerce(arglist->args[1], ctx);
	if(vector2->type == VAL_ERR) {
		Value_free(vector1);
		return vector2;
	}
	
	if(vector1->type != VAL_VEC || vector2->type != VAL_VEC) {
		/* Both values must be vectors */
		ret = ValErr(typeError("Builtin 'dot' expects two vectors."));
	}
	else {
		ret = Vector_dot(vector1->vec, vector2->vec, ctx);
	}
	
	Value_free(vector1);
	Value_free(vector2);
	
	return ret;
}

static Value* eval_cross(Context* ctx, ArgList* arglist, bool internal) {
	if(arglist->count != 2) {
		/* Two vectors are required for a cross product */
		return ValErr(builtinArgs("cross", 2, arglist->count));
	}
	
	Value* vector1 = Value_coerce(arglist->args[0], ctx);
	if(vector1->type == VAL_ERR) {
		return vector1;
	}
	
	Value* vector2 = Value_coerce(arglist->args[1], ctx);
	if(vector2->type == VAL_ERR) {
		Value_free(vector1);
		return vector2;
	}
	
	if(vector1->type != VAL_VEC || vector2->type != VAL_VEC) {
		/* Both values must be vectors */
		Value_free(vector1);
		Value_free(vector2);
		return ValErr(typeError("Builtin 'cross' expects two vectors."));
	}
	
	if(vector1->vec->vals->count != 3 || vector2->vec->vals->count != 3) {
		/* Vectors must each have a size of 3 */
		Value_free(vector1);
		Value_free(vector2);
		return ValErr(mathError("Vectors must each have a size of 3 for cross product."));
	}
	
	
}

static Value* eval_map(Context* ctx, ArgList* arglist, bool internal) {
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
	
	Value* vec = Value_coerce(arglist->args[1], ctx);
	if(vec->type == VAL_ERR) {
		Value_free(func);
		return vec;
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
		Value* call = ValCall(FuncCall_create(func->name, arg));
		
		mapping->args[i] = call;
	}
	
	Value_free(func);
	Value_free(vec);
	
	return ValVec(Vector_new(mapping));
}

static Value* eval_elem(Context* ctx, ArgList* arglist, bool internal) {
	if(arglist->count != 2) {
		return ValErr(builtinArgs("elem", 2, arglist->count));
	}
	
	/* Get evaluated values */
	Value* vec = Value_coerce(arglist->args[0], ctx);
	Value* index = Value_coerce(arglist->args[1], ctx);
	
	/* Check vector type */
	if(vec->type != VAL_VEC) {
		return ValErr(typeError("Only vectors are subscriptable."));
	}
	
	/* Get actual value */
	Value* ret = Vector_elem(vec->vec, index, ctx);
	
	/* Free allocated memory */
	Value_free(vec);
	Value_free(index);
	return ret;
}

static Value* eval_mag(Context* ctx, ArgList* arglist, bool internal) {
	if(arglist->count != 1) {
		return ValErr(builtinArgs("mag", 1, arglist->count));
	}
	
	Value* vec = Value_eval(arglist->args[0], ctx);
	if(vec->type == VAL_ERR) {
		return vec;
	}
	
	if(vec->type != VAL_VEC) {
		Value_free(vec);
		return ValErr(typeError("Can only evaluate the magnitude of a vector."));
	}
	
	return Vector_magnitude(vec->vec, ctx);
}

static Value* eval_norm(Context* ctx, ArgList* arglist, bool internal) {
	if(arglist->count != 1) {
		return ValErr(builtinArgs("norm", 1, arglist->count));
	}
	
	Value* val = Value_eval(arglist->args[0], ctx);
	if(val->type != VAL_VEC) {
		Value_free(val);
		return ValErr(typeError("Can only normalize a vector."));
	}
	
	return ValExpr(BinOp_new(BIN_DIV, val, Vector_magnitude(val->vec, ctx)));
}

static const char* _vector_names[] = {
	"dot", "cross", "map",
	"elem", "mag", "norm"
};
static builtin_eval_t _vector_funcs[] = {
	&eval_dot, &eval_cross, &eval_map,
	&eval_elem, &eval_mag, &eval_norm
};

/* This is just a copy of register_math remade for vectors */
void Vector_register(Context* ctx) {
	unsigned count = sizeof(_vector_names) / sizeof(_vector_names[0]);
	unsigned i;
	for(i = 0; i < count; i++) {
		Builtin* blt = Builtin_new(_vector_names[i], _vector_funcs[i]);
		Builtin_register(blt, ctx);
	}
}

