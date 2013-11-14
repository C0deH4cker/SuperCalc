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
		
		if(val->type == VAL_END) {
			Value_free(val);
			var = VarErr(ignoreError());
		}
		else if(val->type == VAL_ERR) {
			var = VarErr(Error_copy(val->err));
			Value_free(val);
		}
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
			var = VarErr(earlyEnd());
			return Expression_new(var);
		}
		
		/* Now parse the left side */
		char* name = nextToken(expr);
		if(name == NULL) {
			Value_free(val);
			var = VarErr(syntaxError("No variable to assign to."));
			return Expression_new(var);
		}
		
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
				if(**expr == ')') {
					/* Empty parameter list means function with no args */
					free(args);
					var = VarFunc(name, Function_new(0, NULL, val));
					free(name);
					
					return Expression_new(var);
				}
				
				/* Invalid character */
				Value_free(val);
				free(args);
				free(name);
				
				var = VarErr(badChar(**expr));
				return Expression_new(var);
			}
			
			trimSpaces(expr);
			while(**expr == ',' || **expr == ')') {
				args[len++] = arg;
				
				if(**expr == ')')
					break;
				
				(*expr)++;
				
				/* Expand argument array if it's too small */
				if(len >= size) {
					size *= 2;
					args = frealloc(args, size);
				}
				
				arg = nextToken(expr);
				if(arg == NULL) {
					/* Invalid character */
					Value_free(val);
					free(name);
					/* Free argument names and return */
					unsigned i;
					for(i = 0; i < len; i++) {
						free(args[i]);
					}
					free(args);
					
					var = VarErr(badChar(**expr));
					return Expression_new(var);
				}
				
				trimSpaces(expr);
			}
			
			if(**expr != ')') {
				/* Invalid character inside argument name list */
				Value_free(val);
				free(name);
				
				/* Free argument names and return */
				unsigned i;
				for(i = 0; i < len; i++) {
					free(args[i]);
				}
				free(args);
				
				var = VarErr(badChar(**expr));
				return Expression_new(var);
			}
			
			/* Skip closing parenthesis */
			(*expr)++;
			trimSpaces(expr);
			
			if(**expr != '=') {
				Value_free(val);
				free(name);
				
				unsigned i;
				for(i = 0; i < len; i++) {
					free(args[i]);
				}
				free(args);
				
				var = VarErr(badChar(**expr));
				return Expression_new(var);
			}
			
			/* Construct function and return it */
			Function* func = Function_new(len, args, val);
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
		/* Variable assignment */
		if(var->val->type == VAL_VAR) {
			/* Right side is a variable */
			Variable* func = Variable_get(ctx, var->val->name);
			if(func == NULL)
				return ValErr(varNotFound(var->val->name));
			
			if(func->type != VAR_BUILTIN) {
				if(var->name != NULL)
					Context_set(ctx, var->name, func);
				
				return ValVar(var->val->name);
			}
		}
		
		/* Right side is an expression */
		ret = Value_eval(var->val, ctx);
		
		/* If an error occurred, bail */
		if(ret->type == VAL_ERR)
			return ret;
		
		/* This means ret must be a Value */
		Value_free(var->val);
		var->val = Value_copy(ret);
		
		/* Update ans */
		Context_set(ctx, "ans", var);
		
		/* Save the newly evaluated variable */
		if(var->name != NULL)
			Context_set(ctx, var->name, var);
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

bool Expression_didError(Expression* expr) {
	return (expr->var->type == VAR_ERR);
}

char* Expression_verbose(Expression* expr, Context* ctx) {
	/* What an if statement!!! */
	if(expr->var->type == VAR_VALUE && expr->var->val->type == VAL_VAR) {
		/* Return the verbose representation of the variable in ctx */
		Variable* var = Variable_get(ctx, expr->var->val->name);
		return Variable_verbose(var);
	}
	
	return Variable_verbose(expr->var);
}

char* Expression_repr(Expression* expr, Context* ctx) {
	/* And another crazy if statement!!! */
	if(expr->var->type == VAR_VALUE && expr->var->val->type == VAL_VAR) {
		/* Return the reprint of the variable in ctx */
		Variable* var = Variable_get(ctx, expr->var->val->name);
		return Variable_repr(var);
	}
	
	return Variable_repr(expr->var);
}

void Expression_print(Expression* expr, Context* ctx, int verbosity) {
	/* Error parsing? */
	if(Expression_didError(expr)) {
		Error_raise(expr->var->err);
		return;
	}
	
	if(verbosity >= 2) {
		/* Dump expression tree */
		char* tree = Expression_verbose(expr, ctx);
		printf("%s\n", tree);
		free(tree);
	}
	
	if(verbosity >= 1) {
		/* Print parenthesized expression */
		char* reprinted = Expression_repr(expr, ctx);
		printf("%s\n", reprinted);
		free(reprinted);
	}
}

