/*
  unop.c
  SuperCalc

  Created by C0deH4cker on 11/6/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "unop.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"
#include "generic.h"
#include "context.h"
#include "value.h"


static long long fact(long long n) {
	long long ret = 1;
	
	while(n > 1)
		ret *= n--;
	
	return ret;
}

static Value* unop_fact(Context* ctx, Value* a) {
	Value* ret;
	
	if(a->type != VAL_INT) {
		return ValErr(typeError("Factorial operand must be an integer."));
	}
	
	if(a->ival > 20) {
		return ValErr(mathError("Factorial operand too large (%lld > 20).", a->ival));
	}
	
	ret = ValInt(fact(a->ival));
	
	return ret;
}


typedef Value* (*unop_t)(Context*, Value*);
static unop_t unop_table[] = {
	&unop_fact
};


UnOp* UnOp_new(UNTYPE type, Value* a) {
	UnOp* ret = fmalloc(sizeof(*ret));
	
	ret->type = type;
	ret->a = a;
	
	return ret;
}

void UnOp_free(UnOp* term) {
	if(!term) return;
	
	if(term->a) Value_free(term->a);
	
	free(term);
}

UnOp* UnOp_copy(UnOp* term) {
	return UnOp_new(term->type, Value_copy(term->a));
}

Value* UnOp_eval(UnOp* term, Context* ctx) {
	if(!term) return ValErr(nullError());
	
	Value* a = Value_coerce(term->a, ctx);
	if(a->type == VAL_ERR)
		return a;
	
	Value* ret = unop_table[term->type](ctx, a);
	
	Value_free(a);
	
	return ret;
}

static char UnOp_getChar(UNTYPE type) {
	switch(type) {
		case UN_FACT:
			return '!';
		
		default:
			/* Shouldn't be reached */
			return '?';
	}
}

char* UnOp_verbose(UnOp* term, int indent) {
	char* ret;
	
	if(!term) return NULL;
	
	char* spacing = spaces(indent + IWIDTH);
	char* current = spaces(indent);
	char* a = Value_verbose(term->a, indent + IWIDTH);
	
	asprintf(&ret, "%c (\n%s%s\n%s)", UnOp_getChar(term->type),
			 spacing, a,
			 current);
	
	free(spacing);
	free(current);
	free(a);
	
	return ret;
}

char* UnOp_repr(UnOp* term) {
	char* ret;
	
	if(term == NULL)
		return strNULL();
	
	char* val = Value_repr(term->a);
	
	switch(term->type) {
		case UN_FACT:
			asprintf(&ret, "%s%c", val, UnOp_getChar(term->type));
			break;
			
		default:
			/* Shouldn't be reached */
			DIE("Unknown unary operator type: %d.", term->type);
	}
	
	free(val);
	
	return ret;
}

