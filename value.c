/*
  value.c
  SuperCalc

  Created by C0deH4cker on 10/21/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <float.h>

#include "error.h"
#include "generic.h"
#include "unop.h"
#include "binop.h"
#include "function.h"
#include "arglist.h"


static Value* allocValue(VALTYPE type);
static void treeAddValue(BinOp** tree, BinOp** prev, BINTYPE op, Value* val);
static Value* parseNum(const char** expr);
static Value* parseToken(const char** expr);


static Value* allocValue(VALTYPE type) {
	Value* ret = fmalloc(sizeof(*ret));
	
	memset(ret, 0, sizeof(*ret));
	
	ret->type = type;
	
	return ret;
}

Value* ValEnd(void) {
	return allocValue(VAL_END);
}

Value* ValErr(Error* err) {
	Value* ret = allocValue(VAL_ERR);
	
	ret->err = err;
	
	return ret;
}

Value* ValNeg(void) {
	return allocValue(VAL_NEG);
}

Value* ValInt(long long val) {
	Value* ret = allocValue(VAL_INT);
	
	ret->ival = val;
	
	return ret;
}

Value* ValReal(double val) {
	Value* ret = allocValue(VAL_REAL);
	
	ret->rval = val;
	
	return ret;
}

Value* ValFrac(Fraction* frac) {
	Value* ret = allocValue(VAL_FRAC);
	
	ret->frac = frac;
	
	Fraction_reduce(ret);
	
	return ret;
}

Value* ValExpr(BinOp* expr) {
	Value* ret = allocValue(VAL_EXPR);
	
	ret->expr = expr;
	
	return ret;
}

Value* ValUnary(UnOp* term) {
	Value* ret = allocValue(VAL_UNARY);
	
	ret->term = term;
	
	return ret;
}

Value* ValCall(FuncCall* call) {
	Value* ret = allocValue(VAL_CALL);
	
	ret->call = call;
	
	return ret;
}

Value* ValVar(const char* name) {
	Value* ret = allocValue(VAL_VAR);
	
	ret->name = strdup(name);
	
	return ret;
}

void Value_free(Value* val) {
	if(!val) return;
	
	switch(val->type) {
		case VAL_EXPR:
			BinOp_free(val->expr);
			break;
			
		case VAL_UNARY:
			UnOp_free(val->term);
			break;
			
		case VAL_CALL:
			FuncCall_free(val->call);
			break;
			
		case VAL_FRAC:
			Fraction_free(val->frac);
			break;
			
		case VAL_VAR:
			free(val->name);
			break;
		
		case VAL_ERR:
			Error_free(val->err);
			break;
			
		default:
			/* The rest don't need to be freed */
			break;
	}
	
	free(val);
}

Value* Value_copy(Value* val) {
	Value* ret;
	
	switch(val->type) {
		case VAL_INT:
			ret = ValInt(val->ival);
			break;
		
		case VAL_REAL:
			ret = ValReal(val->rval);
			break;
		
		case VAL_FRAC:
			ret = ValFrac(Fraction_copy(val->frac));
			break;
		
		case VAL_EXPR:
			ret = ValExpr(BinOp_copy(val->expr));
			break;
		
		case VAL_CALL:
			ret = ValCall(FuncCall_copy(val->call));
			break;
		
		case VAL_UNARY:
			ret = ValUnary(UnOp_copy(val->term));
			break;
		
		case VAL_VAR:
			ret = ValVar(val->name);
			break;
		
		case VAL_NEG:
			/* Shouldn't be reached, but so easy to code */
			ret = ValNeg();
			break;
		
		case VAL_ERR:
			ret = ValErr(Error_copy(val->err));
			break;
		
		default:
			typeError("Unknown value type: %d.", val->type);
			ret = NULL;
			break;
	}
	
	return ret;
}

Value* Value_eval(Value* val, Context* ctx) {
	if(val == NULL) return ValErr(nullError());
	
	Value* ret;
	
	switch(val->type) {
		/* These can be evaluated to a simpler form */
		case VAL_EXPR:
			ret = BinOp_eval(val->expr, ctx);
			break;
		
		case VAL_UNARY:
			ret = UnOp_eval(val->term, ctx);
			break;
		
		case VAL_CALL:
			ret = FuncCall_eval(val->call, ctx);
			break;
		
		case VAL_FRAC:
			ret = Value_copy(val);
			Fraction_reduce(ret);
			break;
		
		case VAL_VAR:
			ret = Variable_eval(val->name, ctx);
			break;
		
		/* These can't be simplified, so just copy them */
		case VAL_INT:
		case VAL_REAL:
		case VAL_NEG:
		case VAL_ERR:
			ret = Value_copy(val);
			break;
		
		default:
			/* Shouldn't be reached */
			badValType(val->type);
	}
	
	return ret;
}

double Value_asReal(Value* val) {
	double ret;
	
	switch(val->type) {
		case VAL_INT:
			ret = val->ival;
			break;
		
		case VAL_REAL:
			ret = val->rval;
			break;
		
		case VAL_FRAC:
			ret = Fraction_asReal(val->frac);
			break;
		
		default:
			/* Expression couldn't be evaluated, so it's not a number */
			ret = NAN;
			break;
	}
	
	return ret;
}

Value* Value_parse(const char** expr, char sep, char end) {
	Value* val;
	BINTYPE op = BIN_UNK;
	BinOp* tree = NULL;
	BinOp* prev;
	
	trimSpaces(expr);
	if(**expr == '\0')
		return ValEnd();
	
	while(1) {
		/* Get next value */
		val = Value_next(expr);
		
		/* Error parsing next value? */
		if(val->type == VAL_ERR) {
			if(tree) BinOp_free(tree);
			
			return val;
		}
		
		/* End of input? */
		if(val->type == VAL_END) {
			if(tree) BinOp_free(tree);
			
			return val;
		}
		
		/* Special case: negative value */
		if(val->type == VAL_NEG) {
			/* XXX: Is this right? */
			BinOp* cur = BinOp_new(BIN_MUL, ValInt(-1), NULL);
			
			if(!tree) {
				tree = cur;
			}
			else {
				prev->b = ValExpr(cur);
			}
			
			prev = cur;
			continue;
		}
		
		/* Get next operator if it exists */
		op = BinOp_nextType(expr, sep, end);
		
		/* Invalid operator? Return syntax error */
		if(op == BIN_UNK) {
			/* Exit gracefully and return error */
			if(tree) BinOp_free(tree);
			
			/* XXX: Should val be freed here or not? */
			Value_free(val);
			
			return ValErr(badChar(**expr));
		}
		/* End of the expression? */
		else if(op == BIN_END) {
			if(**expr == sep)
				(*expr)++;
			
			/* If there was only one value, return it */
			if(!tree)
				return val;
			
			/* Otherwise, place the final value into the tree and break out of the parse loop */
			prev->b = val;
			break;
		}
		
		/* Tree not yet begun? Initialize it! */
		if(tree == NULL) {
			tree = BinOp_new(op, val, NULL);
			prev = tree;
		}
		else {
			/* Tree already started, so add to it */
			treeAddValue(&tree, &prev, op, val);
		}
	}
	
	return ValExpr(tree);
}

static void treeAddValue(BinOp** tree, BinOp** prev, BINTYPE op, Value* val) {
	BinOp* parent = *tree;
	BinOp* cur = *tree;
	BinOp* next;
	
	/*
	 Descend down the right side of the tree until we find either an empty node or
	 an operator with a higher precedence than the current one.
	 */
	while(cur->b && BinOp_cmp(cur->type, op) < 0) {
		parent = cur;
		cur = cur->b->expr;
	}
	
	if(BinOp_cmp(cur->type, op) >= 0) {
		/* Replace current node with new one */
		if(cur == *tree) {
			/* At the tree's root */
			next = BinOp_new(op, ValExpr(*tree), NULL);
			*tree = next;
		}
		else {
			/* Somewhere in the tree */
			next = BinOp_new(op, parent->b, NULL);
			parent->b = ValExpr(next);
		}
		(*prev)->b = val;
	}
	else {
		/* New node is child of current node */
		next = BinOp_new(op, val, NULL);
		(*prev)->b = ValExpr(next);
	}
	
	*prev = next;
}

static Value* parseNum(const char** expr) {
	Value* ret;
	
	char* end1;
	char* end2;
	
	errno = 0;
	
	double dbl = strtod(*expr, &end1);
	if(errno != 0) {
		/* An error occurred (EINVAL, ERANGE) */
		end1 = NULL;
	}
	if(*expr == end1) {
		/* Nothing got parsed */
		end1 = NULL;
	}
	
	long long ll = strtoll(*expr, &end2, 10);
	if(errno != 0) {
		/* An error occurred (EINVAL, ERANGE) */
		end2 = NULL;
	}
	if(*expr == end2) {
		/* Nothing got parsed */
		end2 = NULL;
	}
	
	
	if(end1 > end2) {
		/* Must be a double because more of the string was parsed as double than long long */
		ret = ValReal(dbl);
		*expr = end1;
	}
	else if(end2 >= end1 && end2 != NULL) {
		/* Must be an integer */
		ret = ValInt(ll);
		*expr = end2;
	}
	else {
		/* Both failed to convert the data */
		ret = ValErr(badChar(**expr));
	}
	
	return ret;
}

static Value* parseToken(const char** expr) {
	Value* ret;
	
	char* token = nextToken(expr);
	trimSpaces(expr);
	
	if(token == NULL)
		return ValErr(badChar(**expr));
	
	/* TODO: Handle nested calls like getFunc()(4) */
	if(**expr == '(') {
		(*expr)++;
		ArgList* arglist = ArgList_parse(expr, ',', ')');
		FuncCall* call = FuncCall_new(token, arglist);
		
		ret = ValCall(call);
	}
	else {
		ret = ValVar(token);
	}
	
	free(token);
	
	return ret;
}

Value* Value_next(const char** expr) {
	Value* ret;
	
	if(getSign(expr) == -1) return ValNeg();
	
	trimSpaces(expr);
	
	if(isdigit(**expr) || **expr == '.') {
		ret = parseNum(expr);
	}
	else if(**expr == '(') {
		(*expr)++;
		ret = Value_parse(expr, 0, 0);
		
		/* Skip closing parenthesis if it exists */
		if(**expr == ')')
			(*expr)++;
	}
	else if(**expr == ')') {
		(*expr)++;
		return ValEnd();
	}
	else {
		ret = parseToken(expr);
	}
	
	/* Check if a parse error occurred */
	if(ret->type == VAL_ERR)
		return ret;
	
	/* Check for unary sign(s) afterwards */
	trimSpaces(expr);
	while(**expr == '!') {
		(*expr)++;
		trimSpaces(expr);
		ret = ValUnary(UnOp_new(UN_FACT, ret));
	}

	return ret;
}

char* Value_verbose(Value* val, int indent) {
	char* ret;
	
	switch(val->type) {
		case VAL_INT:
			asprintf(&ret, "%lld", val->ival);
			break;
		
		case VAL_REAL:
			asprintf(&ret, "%.*g", DBL_DIG, val->rval);
			break;
		
		case VAL_FRAC:
			ret = Fraction_repr(val->frac);
			break;
		
		case VAL_UNARY:
			ret = UnOp_verbose(val->term, indent);
			break;
		
		case VAL_EXPR:
			ret = BinOp_verbose(val->expr, indent);
			break;
		
		case VAL_CALL:
			ret = FuncCall_verbose(val->call, indent);
			break;
		
		case VAL_VAR:
			ret = strdup(val->name);
			break;
		
		default:
			badValType(val->type);
	}
	
	return ret;
}

char* Value_repr(Value* val) {
	char* ret;
	char* str;
	
	if(val == NULL) {
		asprintf(&ret, "NULL");
		return ret;
	}
	
	switch(val->type) {
		case VAL_INT:
			asprintf(&ret, "%lld", val->ival);
			break;
		
		case VAL_REAL:
			asprintf(&ret, "%.*g", DBL_DIG, val->rval);
			break;
		
		case VAL_FRAC:
			ret = Fraction_repr(val->frac);
			break;
		
		case VAL_UNARY:
			str = UnOp_repr(val->term);
			asprintf(&ret, "(%s)", str);
			free(str);
			break;
		
		case VAL_EXPR:
			str = BinOp_repr(val->expr);
			asprintf(&ret, "(%s)", str);
			free(str);
			break;
		
		case VAL_CALL:
			ret = FuncCall_repr(val->call);
			break;
		
		case VAL_VAR:
			ret = strdup(val->name);
			break;
		
		default:
			/* Shouldn't be reached */
			badValType(val->type);
	}
	
	return ret;
}

void Value_fprint(FILE* fp, Value* val, Context* ctx) {
	if(val->type == VAL_VAR) {
		Variable* var = Variable_get(ctx, val->name);
		
		/* Don't try to evaluate a function */
		if(var->type == VAR_FUNC)
			return;
		
		val = Variable_eval(val->name, ctx);
	}
	
	if(val->type == VAL_ERR) {
		/* An error occurred, so print it and continue. */
		Error_raise(val->err);
		return;
	}
	
	/* Print the value */
	char* valString = Value_repr(val);
	if(valString) {
		fprintf(fp, "%s", valString);
		free(valString);
	}
	
	/* If the result is a fraction, also print out the floating point representation */
	if(val->type == VAL_FRAC) {
		fprintf(fp, " (%.*g)", DBL_DIG, Fraction_asReal(val->frac));
	}
	
	fputc('\n', fp);
}

void Value_print(Value* val, Context* ctx) {
	Value_fprint(stdout, val, ctx);
}

