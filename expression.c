/*
  expression.c
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "expression.h"
#include <string.h>
#include <stdio.h>

#include "value.h"
#include "variable.h"
#include "context.h"
#include "generic.h"
#include "binop.h"



Expression* Expression_new(Variable* var) {
	Expression* ret = fmalloc(sizeof(*ret));
	
	ret->var = var;
	
	return ret;
}

void Expression_free(Expression* expr) {
	Variable_free(expr->var);
	free(expr);
}

Expression* Expression_parse(const char** expr) {
	Expression* ret;
	Variable* var;
	Value* val;
	
	const char* equals = strchr(*expr, '=');
	
	if(equals == NULL) {
		/* No assignment, just a plain expression. */
		val = Value_parse(expr);
		
		if(val->type == VAL_END)
			var = VarErr(ignoreError());
		else
			var = VarValue(NULL, val);
		
		ret = Expression_new(var);
	}
	else {
		/* There is an assignment */
		/* First, parse the right side of the assignment */
		equals++;
		val = Value_parse(&equals);
		
		if(val->type == VAL_END) {
			/* Empty input */
			Value_free(val);
			var = VarErr(syntaxError("Expected expression."));
			return Expression_new(var);
		}
		
		/* Now parse the left side */
		char* name = nextToken(expr);
		trimSpaces(expr);
		
		if(**expr == '(') {
			/* Defining a function */
			(*expr)++;
			
			/* Array of argument names */
			unsigned size = 2;
			char** args = fmalloc(size * sizeof(*args));
			unsigned len = 0;
			
			/* Add each argument name to the array */
			char* arg = nextToken(expr);
			
			if(arg == NULL) {
				Value_free(val);
				free(args);
				free(name);
				
				var = VarErr(badChar(**expr));
				return Expression_new(var);
			}
			
			trimSpaces(expr);
			while(**expr == ',' || **expr == ')') {
				args[len++] = arg;
				
				if(*(*expr)++ == ')') {
					break;
				}
				
				/* Expand argument array if it's too small */
				if(len >= size) {
					size *= 2;
					args = frealloc(args, size);
				}
				
				arg = nextToken(expr);
				trimSpaces(expr);
			}
			
			trimSpaces(expr);
			
			if(**expr != '=') {
				Value_free(val);
				free(args);
				free(name);
				
				var = VarErr(badChar(**expr));
				return Expression_new(var);
			}
			
			
			Function* func = Function_new(len, (const char* const*)args, val);
			free(args);
			var = VarFunc(name, func);
			free(name);
			
			ret = Expression_new(var);
		}
		else {
			/* Defining a variable */
			if(**expr != '=') {
				/* In-place manipulation */
				BINTYPE bin = BinOp_nextType(expr);
				
				/* Still not an equals sign means invalid character */
				if(**expr != '=') {
					Value_free(val);
					free(name);
					
					var = VarErr(badChar(**expr));
					return Expression_new(var);
				}
				
				val = ValExpr(BinOp_new(bin, ValVar(name), val));
			}
			
			var = VarValue(name, val);
			free(name);
			
			ret = Expression_new(var);
		}
	}
	
	return ret;
}

Value* Expression_eval(Expression* expr, Context* ctx) {
	Value* ret;
	Variable* var = expr->var;
	
	if(var->type == VAR_VALUE) {
		ret = Value_eval(var->val, ctx);
		
		/* As long as there wasn't an error, assign the new values */
		if(ret->type != VAL_ERR) {
			Value_free(var->val);
			var->val = Value_copy(ret);
			
			if(var->name != NULL)
				Context_set(ctx, var->name, var);
			
			Context_set(ctx, "ans", var);
		}
	}
	else if(var->type == VAR_FUNC) {
		ret = ValVar(var->name);
		Context_set(ctx, var->name, var);
	}
	else {
		badVarType(var->type);
	}
	
	return ret;
}

char* Expression_verbose(Expression* expr, int indent) {
	return Variable_verbose(expr->var, indent);
}

char* Expression_repr(Expression* expr) {
	return Variable_repr(expr->var);
}


