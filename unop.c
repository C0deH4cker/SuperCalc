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

typedef Value* (*unop_t)(const Context*, const Value*);

static long long fact(long long n);
static Value* unop_fact(const Context* ctx, const Value* a);

static unop_t _unop_table[] = {
	&unop_fact
};
static const char* _unop_repr[] = {
	"!"
};
static const char* _unop_xml[] = {
	"fact"
};

static long long fact(long long n) {
	long long ret = 1;
	
	while(n > 1) {
		ret *= n--;
	}
	
	return ret;
}

static Value* unop_fact(const Context* ctx, const Value* a) {
	if(a->type != VAL_INT) {
		return ValErr(typeError("Factorial operand must be an integer."));
	}
	
	if(a->ival > 20) {
		return ValErr(mathError("Factorial operand too large (%lld > 20).", a->ival));
	}
	
	return ValInt(fact(a->ival));
}

UnOp* UnOp_new(UNTYPE type, Value* a) {
	UnOp* ret = fmalloc(sizeof(*ret));
	
	ret->type = type;
	ret->a = a;
	
	return ret;
}

void UnOp_free(UnOp* term) {
	if(!term) return;
	
	if(term->a) {
		Value_free(term->a);
	}
	
	free(term);
}

UnOp* UnOp_copy(const UnOp* term) {
	return UnOp_new(term->type, Value_copy(term->a));
}

Value* UnOp_eval(const UnOp* term, const Context* ctx) {
	if(!term) {
		return ValErr(nullError());
	}
	
	Value* a = Value_coerce(term->a, ctx);
	if(a->type == VAL_ERR) {
		return a;
	}
	
	Value* ret = _unop_table[term->type](ctx, a);
	
	Value_free(a);
	return ret;
}

char* UnOp_repr(const UnOp* term, bool pretty) {
	char* ret;
	char* val = Value_repr(term->a, pretty);
	
	switch(term->type) {
		case UN_FACT:
			asprintf(&ret, "%s%s", val, _unop_repr[term->type]);
			break;
			
		default:
			/* Shouldn't be reached */
			DIE("Unknown unary operator type: %d.", term->type);
	}
	
	free(val);
	return ret;
}

char* UnOp_verbose(const UnOp* term, unsigned indent) {
	char* ret;
	char* a = Value_verbose(term->a, indent + 1);
	
	asprintf(&ret,
			 "%3$s (\n"       /* Type */
				 "%2$s%4$s\n" /* a */
			 "%1$s)",
			 indentation(indent), indentation(indent + 1),
			 _unop_repr[term->type],
			 a);
	
	free(a);
	return ret;
}

char* UnOp_xml(const UnOp* term, unsigned indent) {
	/*
	 sc> ?x (3 - 1)!
	 
	 <fact>
	   <sub>
	     <int>3</int>
	     <int>1</int>
	   </sub>
	 </fact>
	 
	 2
	*/
	char* ret;
	char* a = Value_xml(term->a, indent + 1);
	
	asprintf(&ret,
			 "<%1$s>\n"
				 "%3$s%4$s\n" /* a */
			 "%2$s</%1$s>",
			 _unop_xml[term->type],
			 indentation(indent), indentation(indent + 1),
			 a);
	
	free(a);
	return ret;
}

