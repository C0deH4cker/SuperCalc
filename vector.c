//
//  vector.c
//  SuperCalc
//
//  Created by Silas Schwarz on 11/16/13.
//  Copyright (c) 2013 Silas Schwarz. All rights reserved.
//

#include <stdio.h>
#include "vector.h"
#include "generic.h"
#include <ctype.h>

#define type_chk(value, _type) if (value->type != _type) { RAISE(typeError("Input type %d.", ""));
#define chk_end }

Value *Vector_new(Value *count, Value **args) {
    type_chk(count, VAL_INT) return NULL; chk_end // count must be an integer.
    Vector *vector_ret = fmalloc(sizeof(*vector_ret));
    vector_ret->vals->count = (unsigned)count->ival;
    Value_free(count); // I said it would be consumed
    if (!args) { // account for null 'args'
        Value *zero = ValInt(0);
        for (long long i = 0; i < count->ival; i++) {
            vector_ret->vals->args[i] = Value_copy(zero); // copy args so that they can be changed independently
        }
    } else {
        vector_ret->vals->args = args; // I said they would be consumed
    }
    Value *value_ret = fmalloc(sizeof(*value_ret)); // wrap the vector in a 'Value'
    value_ret->type = VAL_VEC;
    value_ret->vec = vector_ret;
    return value_ret;
}

Value *Vector_copy(Value *vector) {
    Vector *new_vector = fmalloc(sizeof(*vector));
    new_vector->vals->count = vector->vec->vals->count;
    new_vector->vals->args = fmalloc(sizeof(Value)*new_vector->vals->count);
    for (int i = 0; i < new_vector->vals->count; i++) {
        new_vector->vals->args[i] = Value_copy(vector->vec->vals->args[i]);
    }
    Value *ret = fmalloc(sizeof(*ret));
    ret->type = VAL_VEC;
    ret->vec = new_vector;
    return ret;
}

void Vector_free(Vector *vector) {
    for (long long i = 0; i < vector->vals->count; i++) {
        Value_free(vector->vals->args[i]); // free all args
    }
    free(vector); // free the vector itself
}

Value* eval_dot(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		return ValErr(builtinArgs("dot", 2, arglist->count)); // two vectors are required for dot product
	}
    Value *vector1 = Value_eval(arglist->args[0], ctx);
    Value *vector2 = Value_eval(arglist->args[1], ctx);
    type_chk(vector1, VAL_VEC) return NULL; chk_end // vectors are required
    type_chk(vector2, VAL_VEC) return NULL; chk_end
    if (vector1->vec->vals->count != vector2->vec->vals->count) { // check number of count
        return ValErr(mathError("Vectors must have the same number of count for dot product: %d != %d.", vector1->vec->vals->count, vector2->vec->vals->count));
    }
    Value *total = ValInt(0); // store the total value of the dot product
    for (long long i = 0; i < vector1->vec->vals->count; i++) {
        BinOp *mul = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[i]), Value_copy(vector2->vec->vals->args[i])); // multiply v1.x and v2.x
        Value *part = BinOp_eval(mul, ctx);
        BinOp *add = BinOp_new(BIN_ADD, part, total); // add v1.x*v2.x to the total dot product
        total = BinOp_eval(add, ctx);
    }
	return total;
}
Value* eval_cross(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		return ValErr(builtinArgs("cross", 2, arglist->count)); // two vectors are required for cross product
	}
    Value *vector1 = Value_eval(arglist->args[0], ctx);
    Value *vector2 = Value_eval(arglist->args[1], ctx);
    type_chk(vector1, VAL_VEC) return NULL; chk_end // vectors are required
    type_chk(vector2, VAL_VEC) return NULL; chk_end;
    if (vector1->vec->vals->count != vector2->vec->vals->count) { // check number of count
        return ValErr(mathError("Vectors must have the same number of count for cross product: %d != %d.", vector1->vec->vals->count, vector2->vec->vals->count));
    }
    if (vector1->vec->vals->count != 3) {
        return ValErr(mathError("Vectors must have three count for cross product. %d != 3", vector1->vec->vals->count));
    }
    BinOp *i_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[1]), Value_copy(vector2->vec->vals->args[2]));
    BinOp *i_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[2]), Value_copy(vector2->vec->vals->args[1]));
    Value *i_pos = BinOp_eval(i_pos_op, ctx);
    Value *i_neg = BinOp_eval(i_neg_op, ctx);
    BinOp_free(i_pos_op);
    BinOp_free(i_neg_op);
    BinOp *i_op = BinOp_new(BIN_SUB, i_pos, i_neg);
    Value *i = BinOp_eval(i_op, ctx);
    BinOp_free(i_op);
    
    BinOp *j_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[0]), Value_copy(vector2->vec->vals->args[2]));
    BinOp *j_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[2]), Value_copy(vector2->vec->vals->args[0]));
    Value *j_pos = BinOp_eval(j_pos_op, ctx);
    Value *j_neg = BinOp_eval(j_neg_op, ctx);
    BinOp_free(j_pos_op);
    BinOp_free(j_neg_op);
    BinOp *j_op = BinOp_new(BIN_SUB, j_pos, j_neg);
    Value *j = BinOp_eval(j_op, ctx);
    BinOp_free(j_op);
    
    BinOp *k_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[0]), Value_copy(vector2->vec->vals->args[1]));
    BinOp *k_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->vals->args[1]), Value_copy(vector2->vec->vals->args[0]));
    Value *k_pos = BinOp_eval(k_pos_op, ctx);
    Value *k_neg = BinOp_eval(k_neg_op, ctx);
    BinOp_free(k_pos_op);
    BinOp_free(k_neg_op);
    BinOp *k_op = BinOp_new(BIN_SUB, k_pos, k_neg);
    Value *k = BinOp_eval(k_op, ctx);
    BinOp_free(k_op);
    
    Value **args = fmalloc(sizeof(*args)*3);
    args[0] = i;
    args[1] = j;
    args[2] = k;
    Value *cross = Vector_new(ValInt(3), args);
	return cross;
}
static const char* math_names[] = {
	"dot", "cross"
};
static builtin_eval_t math_funcs[] = {
	&eval_dot, &eval_cross
};
// This is just a copy of register_math remade for vectors
void Vector_register(Context *ctx) {
    unsigned count = sizeof(math_names) / sizeof(math_names[0]);
	unsigned i;
	for(i = 0; i < count; i++) {
		Builtin* blt = Builtin_new(math_names[i], math_funcs[i]);
		Builtin_register(blt, ctx);
	}
}

Value *Vector_eval(Value *vector, Context *ctx) {
    Value **new_args = fmalloc((sizeof(*new_args)*vector->vec->vals->count));
    for (long long i = 0; i < vector->vec->vals->count; i++) {
        new_args[i] = Value_eval(vector->vec->vals->args[i], ctx);
    }
    Value *new = Vector_new(ValInt(vector->vec->vals->count), new_args);
    if (!new) {
        return vector;
    }
    return new;
}

char *Vector_repr(Value *vector) {
    type_chk(vector, VAL_VEC) RAISE(typeError("Vector required to print vector.")); return NULL; chk_end
    char *out = fmalloc((sizeof(*out)*(vector->vec->vals->count*20)));
    char *temp = out;
    temp += sprintf(temp, "<");
    for (long long i = 0; i < vector->vec->vals->count; i++) {
        temp += sprintf(temp, (vector->vec->vals->count==(i+1) ? "%s" : "%s,"), Value_repr(vector->vec->vals->args[i]));
    }
    temp += sprintf(temp, ">");
    *temp = '\0';
    return out;
}

Value *_Vector_scalar_op(Value *vector, Value *scalar, Context *ctx, BINTYPE op_type) {
    type_chk(vector, VAL_VEC) return NULL; chk_end
    Value **args = fmalloc((sizeof(*args)*vector->vec->vals->count));
    for (long long i = 0; i < vector->vec->vals->count; i++) {
        BinOp *op = BinOp_new(op_type, Value_copy(vector->vec->vals->args[i]), Value_copy(scalar));
        args[i] = BinOp_eval(op, ctx);
        BinOp_free(op);
    }
    Value_free(scalar);
    return Vector_new(ValInt(vector->vec->vals->count), args);
}
Value *Vector_scalar_multiply(Value *vector, Value *scalar, Context *ctx) {
    return _Vector_scalar_op(Value_eval(vector, ctx), Value_eval(scalar, ctx), ctx, BIN_MUL);
}
Value *Vector_scalar_divide(Value *vector, Value *scalar, Context *ctx) {
    return _Vector_scalar_op(Value_eval(vector, ctx), Value_eval(scalar, ctx), ctx, BIN_DIV);
}
Value *_Vector_comp_op(Value *vector1, Value *vector2, Context *ctx, BINTYPE op_type) {
    type_chk(vector1, VAL_VEC) return NULL; chk_end
    type_chk(vector1, VAL_VEC) return NULL; chk_end
    if (vector1->vec->vals->count != vector2->vec->vals->count) {
        return ValErr(mathError("Component operations must operate on vectors of the same number of count."));
    }
    Value **args = fmalloc((sizeof(*args)*vector1->vec->vals->count));
    for (long long i = 0; i < vector1->vec->vals->count; i++) { // perform the operation on each matching component.
        BinOp *comp_add = BinOp_new(op_type, Value_copy(vector1->vec->vals->args[i]), Value_copy(vector2->vec->vals->args[i]));
        args[i] = BinOp_eval(comp_add, ctx);
        BinOp_free(comp_add);
    }
    return Vector_new(ValInt(vector1->vec->vals->count), args);
}
Value *Vector_comp_add(Value *vector1, Value *vector2, Context *ctx) {
    return _Vector_comp_op(vector1, vector2, ctx, BIN_ADD);
}
Value *Vector_comp_subtract(Value *vector1, Value *vector2, Context *ctx) {
    return _Vector_comp_op(vector1, vector2, ctx, BIN_SUB);
}
Value *Vector_comp_multiply(Value *vector1, Value *vector2, Context *ctx) {
    return _Vector_comp_op(vector1, vector2, ctx, BIN_MUL);
}
Value *Vector_comp_divide(Value *vector1, Value *vector2, Context *ctx) {
    return _Vector_comp_op(vector1, vector2, ctx, BIN_DIV);
}