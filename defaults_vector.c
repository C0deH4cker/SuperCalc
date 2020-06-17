/*
  defaults_vector.c
  SuperCalc

  Created by C0deH4cker on 1/15/14.
  Copyright (c) 2014 C0deH4cker. All rights reserved.
*/

#include "defaults.h"
#include "vector.h"
#include <stdbool.h>

#include "error.h"
#include "value.h"
#include "context.h"
#include "arglist.h"
#include "binop.h"
#include "builtin.h"
#include "template.h"

static Value* eval_dot(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
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

static Value* eval_cross(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
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
		/* Vectors must each have a size of 2 or 3 */
		Value_free(vector1);
		Value_free(vector2);
		return ValErr(mathError("Vectors must each have a size of 3 for cross product."));
	}
	
	Value* ret = Vector_cross(vector1->vec, vector2->vec, ctx);
	
	Value_free(vector1);
	Value_free(vector2);
	return ret;
}

static Value* eval_map(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
	if(arglist->count != 2) {
		return ValErr(builtinArgs("map", 2, arglist->count));
	}
	
	Value* callable = Value_copy(arglist->args[0]);
	if(callable->type != VAL_VAR) {
		Value* val = Value_eval(callable, ctx);
		Value_free(callable);
		
		if(val->type == VAL_ERR) {
			return val;
		}
		
		if(!Value_isCallable(val)) {
			Value_free(val);
			return ValErr(typeError("Builtin 'map' expects a callable as its first argument."));
		}
		
		callable = val;
	}
	
	Value* vec = Value_coerce(arglist->args[1], ctx);
	if(vec->type == VAL_ERR) {
		Value_free(callable);
		return vec;
	}
	
	if(vec->type != VAL_VEC) {
		Value_free(callable);
		Value_free(vec);
		return ValErr(typeError("Builtin 'map' expects a vector as its second argument."));
	}
	
	ArgList* mapping = ArgList_new(vec->vec->vals->count);
	
	unsigned i;
	for(i = 0; i < mapping->count; i++) {
		TP(tp);
		mapping->args[i] = TP_EVAL(tp, ctx, "@@(@@)",
								   callable,
								   Value_copy(vec->vec->vals->args[i]));
	}
	
	Value_free(callable);
	Value_free(vec);
	return ValVec(Vector_new(mapping));
}

static Value* eval_elem(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
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

static Value* eval_mag(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
	if(arglist->count != 1) {
		return ValErr(builtinArgs("mag", 1, arglist->count));
	}
	
	Value* vec = Value_coerce(arglist->args[0], ctx);
	if(vec->type != VAL_VEC) {
		Value_free(vec);
		return ValErr(typeError("Can only evaluate the magnitude of a vector."));
	}
	
	return Vector_magnitude(vec->vec, ctx);
}

static Value* eval_norm(const Context* ctx, const ArgList* arglist, bool internal) {
	UNREFERENCED_PARAMETER(internal);
	
	if(arglist->count != 1) {
		return ValErr(builtinArgs("norm", 1, arglist->count));
	}
	
	Value* val = Value_eval(arglist->args[0], ctx);
	if(val->type != VAL_VEC) {
		Value_free(val);
		return ValErr(typeError("Can only normalize a vector."));
	}
	
	TP(tp);
	return TP_EVAL(tp, ctx, "@1v/mag(@1v)", Vector_copy(val->vec));
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
void register_vector(Context* ctx) {
	unsigned count = ARRSIZE(_vector_names);
	unsigned i;
	for(i = 0; i < count; i++) {
		Builtin* blt = Builtin_new(strdup(_vector_names[i]), _vector_funcs[i], true);
		Builtin_register(blt, ctx);
	}
}

