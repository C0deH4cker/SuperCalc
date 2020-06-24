/*
  statement.c
  SuperCalc

  Created by C0deH4cker on 11/11/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "statement.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "generic.h"
#include "error.h"
#include "value.h"
#include "variable.h"
#include "context.h"
#include "binop.h"
#include "function.h"
#include "binop.h"


Statement* Statement_new(Variable* var) {
	Statement* ret = fmalloc(sizeof(*ret));
	
	ret->var = var;
	
	return ret;
}

void Statement_free(Statement* stmt) {
	if(!stmt) {
		return;
	}
	
	Variable_free(stmt->var);
	destroy(stmt);
}

Statement* Statement_parse(const char** expr) {
	Statement* ret = NULL;
	Variable* var;
	Value* val;
	
	const char* equals = strchr(*expr, '=');
	
	if(equals == NULL) {
		/* No assignment, just a plain expression. */
		val = Value_parseTop(expr);
		if(val->type == VAL_ERR) {
			var = VarErr(val->err);
			val->err = CAST_NONNULL(NULL);
			Value_free(val);
		}
		else {
			var = Variable_new(NULL, val);
		}
		
		return Statement_new(var);
	}
	
	/* There is an assignment */
	equals++;
	
	/* First, parse the left side */
	char* name = nextToken(expr);
	if(name == NULL) {
		return Statement_new(VarErr(syntaxError(*expr, "No variable to assign to.")));
	}
	
	trimSpaces(expr);
	
	if(**expr == '(') {
		/* Defining a function */
		(*expr)++;
		
		Error* err = NULL;
		Function* func = Function_parseArgs(expr, ',', ')', &err);
		if(func == NULL) {
			destroy(name);
			return Statement_new(VarErr(err));
		}
		
		trimSpaces(expr);
		
		if(**expr != '=') {
			destroy(name);
			Function_free(func);
			
			return Statement_new(VarErr(badChar(*expr)));
		}
		
		/* Now, parse the right side of the equals sign (function body) */
		val = Value_parseTop(&equals);
		if(val->type == VAL_ERR) {
			/* A parse error occurred */
			destroy(name);
			Function_free(func);
			
			var = VarErr(Error_copy(val->err));
			Value_free(val);
			return Statement_new(var);
		}
		
		/* Construct function and return it */
		func->body = val;
		Variable* var = Variable_new(name, ValFunc(func));
		name = NULL;
		ret = Statement_new(var);
	}
	else {
		/* Defining a variable */
		BINTYPE bin = BIN_UNK;
		
		if(**expr != '=') {
			/* In-place manipulation */
			bin = BinOp_nextType(expr, 0, 0);
			
			/* Still not an equals sign means invalid character */
			if(**expr != '=') {
				destroy(name);
				return Statement_new(VarErr(badChar(*expr)));
			}
		}
		
		/* Now, parse the right side of the equals sign (variable's expression) */
		val = Value_parseTop(&equals);
		if(val->type == VAL_ERR) {
			/* A parse error occurred */
			destroy(name);
			
			var = VarErr(Error_copy(val->err));
			Value_free(val);
			return Statement_new(var);
		}
		
		/* Is this an in-place binop like "+="? */
		if(bin != BIN_UNK) {
			val = ValExpr(BinOp_new(bin, ValVar(strdup(name)), val));
		}
		
		ret = Statement_new(Variable_new(name, val));
		name = NULL;
	}
	
	return ret;
}

Value* Statement_eval(const Statement* stmt, Context* ctx, VERBOSITY v) {
	Value* ret;
	Variable* var = stmt->var;
	
	/* Evaluate right side */
	ret = Value_eval(var->val, ctx);
	
	/* If an error occurred, bail */
	if(ret->type == VAL_ERR) {
		return ret;
	}
	
	/* Statement result is a variable? */
	if(ret->type == VAL_VAR) {
		Variable* func = Variable_get(ctx, ret->name);
		if(func == NULL) {
			Error* err = varNotFound(ret->name);
			Value_free(ret);
			return ValErr(err);
		}
		
		if(var->name != NULL) {
			Context_setGlobal(ctx, CAST_NONNULL(var->name), Value_copy(func->val));
		}
		else if((v & (V_REPR|V_TREE|V_XML)) == 0) {
			/* Coerce the variable to a Value */
			Value* val = Variable_eval(func, ctx);
			Value_free(ret);
			ret = val;
		}
	}
	else {
		/* To handle statements like "pi" */
		if(ret->type == VAL_BUILTIN && !ret->blt->isFunction) {
			Value* tmp = Value_coerce(ret, ctx);
			Value_free(ret);
			ret = tmp;
		}
		
		/* Update ans */
		Context_setGlobal(ctx, "ans", Value_copy(ret));
		
		/* Save the newly evaluated variable */
		if(var->name != NULL) {
			Context_setGlobal(ctx, CAST_NONNULL(var->name), Value_copy(ret));
		}
	}
	
	return ret;
}

bool Statement_didError(const Statement* stmt) {
	return (stmt->var->val->type == VAL_ERR);
}

char* Statement_repr(const Statement* stmt, const Context* ctx, bool pretty) {
	if(stmt->var->val->type == VAL_VAR) {
		/* Return the reprint of the variable in ctx */
		Variable* var = Variable_get(ctx, stmt->var->val->name);
		if(var == NULL) {
			/* If the variable doesn't exist, just return its name */
			if(pretty) {
				return strdup(getPretty(stmt->var->val->name));
			}
			
			return strdup(stmt->var->val->name);
		}
		
		return Variable_repr(var, pretty);
	}
	
	return Variable_repr(stmt->var, pretty);
}

char* Statement_wrap(const Statement* stmt, const Context* ctx) {
	if(stmt->var->val->type == VAL_VAR) {
		/* Return the reprint of the variable in ctx */
		Variable* var = Variable_get(ctx, stmt->var->val->name);
		if(var == NULL) {
			/* If the variable doesn't exist, just return its name */
			return strdup(stmt->var->val->name);
		}
		
		return Variable_wrap(var);
	}
	
	return Variable_wrap(stmt->var);
}

char* Statement_verbose(const Statement* stmt, const Context* ctx) {
	if(stmt->var->val->type == VAL_VAR) {
		/* Return the verbose representation of the variable in ctx */
		Variable* var = Variable_get(ctx, stmt->var->val->name);
		if(var == NULL) {
			/* If the variable doesn't exist, just return its name */
			return Value_verbose(stmt->var->val, 0);
		}
		
		return Variable_verbose(var);
	}
	
	return Variable_verbose(stmt->var);
}

char* Statement_xml(const Statement* stmt, const Context* ctx) {
	if(stmt->var->val->type == VAL_VAR) {
		/* Return the xml representation of the variable in ctx */
		Variable* var = Variable_get(ctx, stmt->var->val->name);
		if(var == NULL) {
			/* If the variable doesn't exist, just return its name */
			return Value_xml(stmt->var->val, 0);
		}
		
		return Variable_xml(var);
	}
	
	return Variable_xml(stmt->var);
}

void Statement_print(const Statement* stmt, const SuperCalc* sc, VERBOSITY v) {
	/* Error parsing? */
	if(Statement_didError(stmt)) {
		return;
	}
	
	int needNewline = 0;
	
	if(v & V_XML) {
		needNewline++;
		
		/* Dump XML output because why not? */
		char* xml = Statement_xml(stmt, sc->ctx);
		printf("%s\n", xml);
		destroy(xml);
	}
	
	if(v & V_TREE) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Dump parse tree */
		char* tree = Statement_verbose(stmt, sc->ctx);
		printf("%s\n", tree);
		destroy(tree);
	}
	
	if(v & V_WRAP) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Wrap lots of stuff in parentheses for clarity */
		char* wrapped = Statement_wrap(stmt, sc->ctx);
		printf("%s\n", wrapped);
		destroy(wrapped);
	}
	
	if(v & V_REPR) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Print parenthesized statement */
		char* reprinted = Statement_repr(stmt, sc->ctx, !!(v & V_PRETTY));
		printf("%s\n", reprinted);
		destroy(reprinted);
	}
	
	if(needNewline++ && sc->interactive) {
		putchar('\n');
	}
}

