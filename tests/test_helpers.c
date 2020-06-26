/*
  test_helpers.c
  sc_tests

  Created by C0deH4cker on 6/24/20.
  Copyright © 2020 C0deH4cker. All rights reserved.
*/

#include "test_helpers.h"

static bool vIsArgList(const ArgList* _Nullable arglist, va_list ap);

bool IsVal(const Value* _Nullable val, ...) {
	va_list ap;
	va_start(ap, val);
	
	bool ret = vIsVal(val, ap);
	
	va_end(ap);
	
	return ret;
}

bool vIsVal(const Value* _Nullable val, va_list ap) {
	if(val == NULL) {
		return false;
	}
	
	VALTYPE type = va_arg(ap, VALTYPE);
	if(val->type != type) {
		if(!(val->type == VAL_REAL && type == VAL_APPROX)) {
			return false;
		}
	}
	
	switch(type) {
		case VAL_INT: {
			long long x = va_arg(ap, long long);
			return val->ival == x;
		}
		
		case VAL_REAL: {
			double x = va_arg(ap, double);
			return val->rval == x;
		}
		
		case VAL_APPROX: {
			double x = va_arg(ap, double);
			double epsilon = va_arg(ap, double);
			return ABS(val->rval - x) < epsilon;
		}
		
		case VAL_FRAC: {
			long long n = va_arg(ap, long long);
			long long d = va_arg(ap, long long);
			return val->frac->n == n && val->frac->d == d;
		}
		
		case VAL_EXPR: {
			BINTYPE bt = va_arg(ap, BINTYPE);
			if(val->expr->type != bt) {
				return false;
			}
			
			if(!vIsVal(val->expr->a, ap)) {
				return false;
			}
			
			return vIsVal(val->expr->b, ap);
		}
		
		case VAL_UNARY: {
			UNTYPE ut = va_arg(ap, UNTYPE);
			if(val->term->type != ut) {
				return false;
			}
			
			return vIsVal(val->term->a, ap);
		}
		
		case VAL_CALL: {
			if(!vIsVal(val->call->func, ap)) {
				return false;
			}
			
			return vIsArgList(val->call->arglist, ap);
		}
		
		case VAL_VAR: {
			const char* name = va_arg(ap, const char*);
			return strcmp(val->name, name) == 0;
		}
		
		case VAL_VEC:
			return vIsArgList(val->vec->vals, ap);
		
		case VAL_FUNC: {
			unsigned argcount = va_arg(ap, unsigned);
			if(val->func->argcount != argcount) {
				return false;
			}
			
			unsigned i;
			for(i = 0; i < argcount; i++) {
				const char* argname = va_arg(ap, const char*);
				if(strcmp(val->func->argnames[i], argname) != 0) {
					return false;
				}
			}
			
			return vIsVal(val->func->body, ap);
		}
		
		case VAL_BUILTIN: {
			const char* name = va_arg(ap, const char*);
			return strcmp(val->blt->name, name) == 0;
		}
		
		case VAL_PLACE: {
			unsigned index = va_arg(ap, unsigned);
			if(val->ph->index != index) {
				return false;
			}
			
			PLACETYPE pt = va_arg(ap, PLACETYPE);
			return val->ph->type == pt;
		}
		
		case VAL_ERR: {
			ERRTYPE et = va_arg(ap, ERRTYPE);
			if(val->err->type != et) {
				return false;
			}
			
			const char* msg = va_arg(ap, const char*);
			return strcmp(val->err->msg, msg) == 0;
		}
		
		default:
			return false;
	}
}

static bool vIsArgList(const ArgList* _Nullable arglist, va_list ap) {
	if(arglist == NULL) {
		return false;
	}
	
	unsigned count = va_arg(ap, unsigned);
	if(arglist->count != count) {
		return false;
	}
	
	unsigned i;
	for(i = 0; i < count; i++) {
		if(!vIsVal(arglist->args[i], ap)) {
			return false;
		}
	}
	
	return true;
}
