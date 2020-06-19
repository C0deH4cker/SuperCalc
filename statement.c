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


Statement* Statement_new(char* assignee, Value* val) {
	Statement* ret = fmalloc(sizeof(*ret));
	
	ret->assignee = assignee;
	ret->val = val;
	
	return ret;
}

void Statement_free(Statement* stmt) {
	if(!stmt) {
		return;
	}
	
	free(stmt->assignee);
	Value_free(stmt->val);
	free(stmt);
}

Statement* Statement_parse(const char** expr) {
	Statement* ret = NULL;
	Value* val;
	
	const char* equals = strchr(*expr, '=');
	
	if(equals == NULL) {
		/* No assignment, just a plain expression. */
		val = Value_parseTop(expr);
		return Statement_new(NULL, val);
	}
	
	/* There is an assignment */
	equals++;
	
	/* First, parse the left side */
	char* name = nextToken(expr);
	if(name == NULL) {
		return Statement_new(NULL, ValErr(syntaxError(*expr, "No variable to assign to.")));
	}
	
	trimSpaces(expr);
	
	if(**expr == '(') {
		/* Defining a function */
		(*expr)++;
		
		Error* err = NULL;
		Function* func = Function_parseArgs(expr, ',', ')', &err);
		if(func == NULL) {
			free(name);
			return Statement_new(NULL, ValErr(err));
		}
		
		trimSpaces(expr);
		
		if(**expr != '=') {
			free(name);
			Function_free(func);
			return Statement_new(NULL, ValErr(badChar(*expr)));
		}
		
		/* Now, parse the right side of the equals sign (function body) */
		val = Value_parseTop(&equals);
		if(val->type == VAL_ERR) {
			/* A parse error occurred */
			free(name);
			Function_free(func);
			return Statement_new(NULL, val);
		}
		
		/* Construct function and return it */
		func->body = val;
		ret = Statement_new(name, ValFunc(func));
		name = NULL;
	}
	else {
		/* Defining a variable */
		BINTYPE bin = BIN_UNK;
		
		if(**expr != '=') {
			/* In-place manipulation */
			bin = BinOp_nextType(expr, 0, 0);
			
			/* Still not an equals sign means invalid character */
			if(**expr != '=') {
				free(name);
				return Statement_new(NULL, ValErr(badChar(*expr)));
			}
		}
		
		/* Now, parse the right side of the equals sign (variable's expression) */
		val = Value_parseTop(&equals);
		if(val->type == VAL_ERR) {
			/* A parse error occurred */
			free(name);
			return Statement_new(NULL, val);
		}
		
		/* Is this an in-place binop like "+="? */
		if(bin != BIN_UNK) {
			val = ValExpr(BinOp_new(bin, ValVar(Variable_new(strdup(name))), val));
		}
		
		ret = Statement_new(name, val);
		name = NULL;
	}
	
	return ret;
}

Value* Statement_eval(const Statement* stmt, Context* ctx) {
	Value* ret;
	
	/* Evaluate right side */
	ret = Value_eval(stmt->val, ctx);
	
	/* If an error occurred, bail */
	if(ret->type == VAL_ERR) {
		return ret;
	}
	
	/* Update ans */
	Context_setGlobal(ctx, "ans", Value_copy(ret));
	
	/* Save the newly evaluated variable */
	if(stmt->assignee != NULL) {
		Context_setGlobal(ctx, stmt->assignee, Value_copy(ret));
	}
	
	return ret;
}

bool Statement_didError(const Statement* stmt) {
	return (stmt->val->type == VAL_ERR);
}

char* Statement_repr(const Statement* stmt, const Context* ctx, bool pretty) {
	if(stmt->assignee == NULL && stmt->val->type == VAL_VAR) {
		/* Return the reprint of the variable in ctx */
		Value* val = Variable_lookup(stmt->val->var, ctx);
		return Variable_repr(stmt->val->var->name, val, pretty);
	}
	
	return Variable_repr(stmt->assignee, stmt->val, pretty);
}

char* Statement_wrap(const Statement* stmt, const Context* ctx) {
	if(stmt->assignee == NULL && stmt->val->type == VAL_VAR) {
		/* Return the reprint of the variable in ctx */
		Value* val = Variable_lookup(stmt->val->var, ctx);
		return Variable_wrap(stmt->val->var->name, val);
	}
	
	return Variable_wrap(stmt->assignee, stmt->val);
}

char* Statement_verbose(const Statement* stmt, const Context* ctx) {
	if(stmt->assignee == NULL && stmt->val->type == VAL_VAR) {
		/* Return the verbose representation of the variable in ctx */
		Value* val = Variable_lookup(stmt->val->var, ctx);
		return Variable_verbose(stmt->val->var->name, val);
	}
	
	return Variable_verbose(stmt->assignee, stmt->val);
}

char* Statement_xml(const Statement* stmt, const Context* ctx) {
	if(stmt->assignee == NULL && stmt->val->type == VAL_VAR) {
		/* Return the xml representation of the variable in ctx */
		Value* val = Variable_lookup(stmt->val->var, ctx);
		return Variable_xml(stmt->val->var->name, val);
	}
	
	return Variable_xml(stmt->assignee, stmt->val);
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
		free(xml);
	}
	
	if(v & V_TREE) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Dump parse tree */
		char* tree = Statement_verbose(stmt, sc->ctx);
		printf("%s\n", tree);
		free(tree);
	}
	
	if(v & V_WRAP) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Wrap lots of stuff in parentheses for clarity */
		char* wrapped = Statement_wrap(stmt, sc->ctx);
		printf("%s\n", wrapped);
		free(wrapped);
	}
	
	if(v & V_REPR) {
		if(needNewline++ && sc->interactive) {
			putchar('\n');
		}
		
		/* Print parenthesized statement */
		char* reprinted = Statement_repr(stmt, sc->ctx, !!(v & V_PRETTY));
		printf("%s\n", reprinted);
		free(reprinted);
	}
	
	if(needNewline++ && sc->interactive) {
		putchar('\n');
	}
}

