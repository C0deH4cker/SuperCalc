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

Value *Vector_new(Value *count, Value **values) {
    type_chk(count, VAL_INT) return NULL; chk_end // count must be an integer.
    struct Vector *vector_ret = fmalloc(sizeof(*vector_ret));
    vector_ret->count = count->ival;
    Value_free(count); // I said it would be consumed
    if (!values) { // account for null 'values'
        Value *zero = ValInt(0);
        for (long long i = 0; i < count->ival; i++) {
            vector_ret->values[i] = Value_copy(zero); // copy values so that they can be changed independently
        }
    } else {
        vector_ret->values = values; // I said they would be consumed
    }
    Value *value_ret = fmalloc(sizeof(*value_ret)); // wrap the vector in a 'Value'
    value_ret->type = VAL_VEC;
    value_ret->vec = vector_ret;
    return value_ret;
}

Value *Vector_copy(Value *vector) {
    struct Vector *new_vector = fmalloc(sizeof(*vector));
    new_vector->count = vector->vec->count;
    new_vector->values = fmalloc(sizeof(Value)*new_vector->count);
    for (int i = 0; i < new_vector->count; i++) {
        new_vector->values[i] = Value_copy(vector->vec->values[i]);
    }
    Value *ret = fmalloc(sizeof(*ret));
    ret->type = VAL_VEC;
    ret->vec = new_vector;
    return ret;
}

void Vector_free(struct Vector *vector) {
    for (long long i = 0; i < vector->count; i++) {
        Value_free(vector->values[i]); // free all values
    }
    free(vector); // free the vector itself
}
#define prc printf("%c\n", **expr);
Value *Vector_parse(const char **expr) {
    size_t size = 2;
    Value **values = fmalloc(size*sizeof(*values));
    unsigned i = 1;
    (*expr) ++;
    values[0] = Value_next(expr);
    while (**expr == ',') {
        (*expr) ++;
        values[i++] = Value_next(expr);
        if (i >= size && **expr == ',') {
            size *= 2;
            values = frealloc(values, (size*sizeof(*values)));
        }
    }
    if (**expr != '>') {
        return ValErr(syntaxError("Closing vector bracket ('>') missing: %c%c%c", *(*expr-1), *(*expr), *(*expr+1)));
    }
    (*expr) ++;
    Value *parsed = Vector_new(ValInt(i), values);
    return (parsed ? parsed : ValErr(syntaxError("Vector parsing failed.")));
}

Value* eval_dot(Context* ctx, ArgList* arglist) {
	if(arglist->count != 2) {
		return ValErr(builtinArgs("dot", 2, arglist->count)); // two vectors are required for dot product
	}
    Value *vector1 = Value_eval(arglist->args[0], ctx);
    Value *vector2 = Value_eval(arglist->args[1], ctx);
    type_chk(vector1, VAL_VEC) return NULL; chk_end // vectors are required
    type_chk(vector2, VAL_VEC) return NULL; chk_end
    if (vector1->vec->count != vector2->vec->count) { // check number of count
        return ValErr(mathError("Vectors must have the same number of count for dot product: %d != %d.", vector1->vec->count, vector2->vec->count));
    }
    Value *total = ValInt(0); // store the total value of the dot product
    for (long long i = 0; i < vector1->vec->count; i++) {
        BinOp *mul = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[i]), Value_copy(vector2->vec->values[i])); // multiply v1.x and v2.x
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
    if (vector1->vec->count != vector2->vec->count) { // check number of count
        return ValErr(mathError("Vectors must have the same number of count for cross product: %d != %d.", vector1->vec->count, vector2->vec->count));
    }
    if (vector1->vec->count != 3) {
        return ValErr(mathError("Vectors must have three count for cross product. %d != 3", vector1->vec->count));
    }
    BinOp *i_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[1]), Value_copy(vector2->vec->values[2]));
    BinOp *i_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[2]), Value_copy(vector2->vec->values[1]));
    Value *i_pos = BinOp_eval(i_pos_op, ctx);
    Value *i_neg = BinOp_eval(i_neg_op, ctx);
    BinOp_free(i_pos_op);
    BinOp_free(i_neg_op);
    BinOp *i_op = BinOp_new(BIN_SUB, i_pos, i_neg);
    Value *i = BinOp_eval(i_op, ctx);
    BinOp_free(i_op);
    
    BinOp *j_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[0]), Value_copy(vector2->vec->values[2]));
    BinOp *j_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[2]), Value_copy(vector2->vec->values[0]));
    Value *j_pos = BinOp_eval(j_pos_op, ctx);
    Value *j_neg = BinOp_eval(j_neg_op, ctx);
    BinOp_free(j_pos_op);
    BinOp_free(j_neg_op);
    BinOp *j_op = BinOp_new(BIN_SUB, j_pos, j_neg);
    Value *j = BinOp_eval(j_op, ctx);
    BinOp_free(j_op);
    
    BinOp *k_pos_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[0]), Value_copy(vector2->vec->values[1]));
    BinOp *k_neg_op = BinOp_new(BIN_MUL, Value_copy(vector1->vec->values[1]), Value_copy(vector2->vec->values[0]));
    Value *k_pos = BinOp_eval(k_pos_op, ctx);
    Value *k_neg = BinOp_eval(k_neg_op, ctx);
    BinOp_free(k_pos_op);
    BinOp_free(k_neg_op);
    BinOp *k_op = BinOp_new(BIN_SUB, k_pos, k_neg);
    Value *k = BinOp_eval(k_op, ctx);
    BinOp_free(k_op);
    
    Value **values = fmalloc(sizeof(*values)*3);
    values[0] = i;
    values[1] = j;
    values[2] = k;
    Value *cross = Vector_new(ValInt(3), values);
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
    Value **new_values = fmalloc((sizeof(*new_values)*vector->vec->count));
    for (long long i = 0; i < vector->vec->count; i++) {
        new_values[i] = Value_eval(vector->vec->values[i], ctx);
    }
    Value *new = Vector_new(ValInt(vector->vec->count), new_values);
    if (!new) {
        return vector;
    }
    return new;
}

char *Vector_repr(Value *vector) {
    type_chk(vector, VAL_VEC) RAISE(typeError("Vector required to print vector.")); return NULL; chk_end
    char *out = fmalloc((sizeof(*out)*(vector->vec->count*20)));
    char *temp = out;
    temp += sprintf(temp, "<");
    for (long long i = 0; i < vector->vec->count; i++) {
        temp += sprintf(temp, (vector->vec->count==(i+1) ? "%s" : "%s,"), Value_repr(vector->vec->values[i]));
    }
    temp += sprintf(temp, ">");
    *temp = '\0';
    return out;
}

Value *_Vector_scalar_op(Value *vector, Value *scalar, Context *ctx, BINTYPE op_type) {
    type_chk(vector, VAL_VEC) return NULL; chk_end
    Value **values = fmalloc((sizeof(*values)*vector->vec->count));
    for (long long i = 0; i < vector->vec->count; i++) {
        BinOp *op = BinOp_new(op_type, Value_copy(vector->vec->values[i]), Value_copy(scalar));
        values[i] = BinOp_eval(op, ctx);
        BinOp_free(op);
    }
    Value_free(scalar);
    return Vector_new(ValInt(vector->vec->count), values);
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
    if (vector1->vec->count != vector2->vec->count) {
        return ValErr(mathError("Component operations must operate on vectors of the same number of count."));
    }
    Value **values = fmalloc((sizeof(*values)*vector1->vec->count));
    for (long long i = 0; i < vector1->vec->count; i++) { // perform the operation on each matching component.
        BinOp *comp_add = BinOp_new(op_type, Value_copy(vector1->vec->values[i]), Value_copy(vector2->vec->values[i]));
        values[i] = BinOp_eval(comp_add, ctx);
        BinOp_free(comp_add);
    }
    return Vector_new(ValInt(vector1->vec->count), values);
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