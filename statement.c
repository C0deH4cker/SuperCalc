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
	Variable_free(stmt->var);
	free(stmt);
}

Statement* Statement_parse(const char** expr) {
	Statement* ret = NULL;
	Variable* var;
	Value* val;
	
	const char* equals = strchr(*expr, '=');
	
	if(equals == NULL) {
		/* No assignment, just a plain expression. */
		val = Value_parse(expr, 0, 0, &default_cb);
		
		if(val->type == VAL_END) {
			Value_free(val);
			var = VarErr(earlyEnd());
		}
		else if(val->type == VAL_ERR) {
			var = VarErr(Error_copy(val->err));
			Value_free(val);
		}
		else {
			var = VarValue(NULL, val);
		}
		
		return Statement_new(var);
	}
	
	/* There is an assignment */
	/* First, parse the right side of the assignment */
	equals++;
	val = Value_parse(&equals, 0, 0, &default_cb);
	
	if(val->type == VAL_ERR) {
		/* A parse error occurred */
		var = VarErr(Error_copy(val->err));
		Value_free(val);
		return Statement_new(var);
	}
	
	if(val->type == VAL_END) {
		/* Empty input */
		Value_free(val);
		return Statement_new(VarErr(earlyEnd()));
	}
	
	/* Now parse the left side */
	char* name = nextToken(expr);
	if(name == NULL) {
		Value_free(val);
		return Statement_new(VarErr(syntaxError("No variable to assign to.")));
	}
	
	trimSpaces(expr);
	
	if(**expr == '(') {
		/* Defining a function */
		/*
		 TODO: Consider utilizing ArgList_parse for this and
		 just check to make sure all args are VAL_VAR
		*/
		(*expr)++;
		
		/* Array of argument names */
		unsigned size = 2;
		char** args = fmalloc(size * sizeof(*args));
		unsigned len = 0;
		
		/* Add each argument name to the array */
		char* arg = nextToken(expr);
		
		if(arg == NULL && **expr != ')') {
			/* Invalid character */
			Value_free(val);
			free(args);
			free(name);
			return Statement_new(VarErr(badChar(**expr)));
		}
		
		trimSpaces(expr);
		
		if(arg == NULL) {
			/* Empty parameter list means function with no args */
			free(args);
			args = NULL;
			len = 0;
		}
		else {
			/* Loop through each argument in the list */
			while(**expr == ',' || **expr == ')') {
				args[len++] = arg;
				arg = NULL;
				
				if(**expr == ')') {
					break;
				}
				
				(*expr)++;
				
				/* Expand argument array if it's too small */
				if(len >= size) {
					size *= 2;
					args = frealloc(args, size * sizeof(*args));
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
					return Statement_new(VarErr(badChar(**expr)));
				}
				
				trimSpaces(expr);
			}
		}
		
		if(arg) {
			free(arg);
		}
		
		if(**expr != ')') {
			/* Invalid character inside argument name list */
			Value_free(val);
			free(name);
			
			if(args) {
				/* Free argument names and return */
				unsigned i;
				for(i = 0; i < len; i++) {
					free(args[i]);
				}
				free(args);
			}
			
			return Statement_new(VarErr(badChar(**expr)));
		}
		
		/* Skip closing parenthesis */
		(*expr)++;
		trimSpaces(expr);
		
		if(**expr != '=') {
			Value_free(val);
			free(name);
			
			if(args) {
				unsigned i;
				for(i = 0; i < len; i++) {
					free(args[i]);
				}
				free(args);
			}
			
			return Statement_new(VarErr(badChar(**expr)));
		}
		
		/* Construct function and return it */
		Function* func = Function_new(len, args, val);
		ret = Statement_new(VarFunc(name, func));
	}
	else {
		/* Defining a variable */
		if(**expr != '=') {
			/* In-place manipulation */
			BINTYPE bin = BinOp_nextType(expr, 0, 0);
			
			/* Still not an equals sign means invalid character */
			if(**expr != '=') {
				Value_free(val);
				free(name);
				return Statement_new(VarErr(badChar(**expr)));
			}
			
			val = ValExpr(BinOp_new(bin, ValVar(name), val));
		}
		
		ret = Statement_new(VarValue(name, val));
	}
	
	return ret;
}

Value* Statement_eval(const Statement* stmt, Context* ctx, VERBOSITY v) {
	Value* ret;
	Variable* var = stmt->var;
	
	if(var->type == VAR_VALUE) {
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
				/* Assign the variable */
				if(func->type == VAR_BUILTIN) {
					Value_free(ret);
					return ValErr(typeError("Cannot assign a variable to a builtin."));
				}
				
				Context_setGlobal(ctx, var->name, Variable_copy(func));
			}
			else if((v & (V_REPR|V_TREE|V_XML)) == 0
					|| (func->type != VAR_FUNC && func->type != VAR_BUILTIN)) {
				/* Coerce the variable to a Value */
				Value* val = Variable_coerce(func, ctx);
				Value_free(ret);
				ret = val;
			}
		}
		else {
			/* This means ret must be a Value */
			Value_free(var->val);
			var->val = Value_copy(ret);
			
			/* Update ans */
			Context_setGlobal(ctx, "ans", Variable_copy(var));
			
			/* Save the newly evaluated variable */
			if(var->name != NULL) {
				Context_setGlobal(ctx, var->name, Variable_copy(var));
			}
		}
	}
	else if(var->type == VAR_FUNC) {
		ret = ValVar(var->name);
		Context_setGlobal(ctx, var->name, Variable_copy(var));
	}
	else {
		badVarType(var->type);
	}
	
	return ret;
}

bool Statement_didError(const Statement* stmt) {
	return (stmt->var->type == VAR_ERR);
}

char* Statement_repr(const Statement* stmt, const Context* ctx, bool pretty) {
	/* I think I toungued my twist trying to read this aloud */
	if(stmt->var->type == VAR_VALUE && stmt->var->val->type == VAL_VAR) {
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
	/* I think I toungued my twist trying to read this aloud */
	if(stmt->var->type == VAR_VALUE && stmt->var->val->type == VAL_VAR) {
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
	if(stmt->var->type == VAR_VALUE && stmt->var->val->type == VAL_VAR) {
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
	if(stmt->var->type == VAR_VALUE && stmt->var->val->type == VAL_VAR) {
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
		Error_raise(stmt->var->err, false);
		return;
	}
	
	int needNewline = 0;
	
	if(v & V_XML) {
		needNewline++;
		
		/* Dump XML output because why not? */
		char* xml = Statement_xml(stmt, sc->ctx);
		fprintf(sc->fout, "%s\n", xml);
		free(xml);
	}
	
	if(v & V_TREE) {
		if(needNewline++ && sc->interactive) {
			fputc('\n', sc->fout);
		}
		
		/* Dump parse tree */
		char* tree = Statement_verbose(stmt, sc->ctx);
		fprintf(sc->fout, "%s\n", tree);
		free(tree);
	}
	
	if(v & V_WRAP) {
		if(needNewline++ && sc->interactive) {
			fputc('\n', sc->fout);
		}
		
		/* Wrap lots of stuff in parentheses for clarity */
		char* wrapped = Statement_wrap(stmt, sc->ctx);
		fprintf(sc->fout, "%s\n", wrapped);
		free(wrapped);
	}
	
	if(v & V_REPR) {
		if(needNewline++ && sc->interactive) {
			fputc('\n', sc->fout);
		}
		
		/* Print parenthesized statement */
		char* reprinted = Statement_repr(stmt, sc->ctx, v & V_PRETTY);
		fprintf(sc->fout, "%s\n", reprinted);
		free(reprinted);
	}
	
	if(needNewline++ && sc->interactive) {
		fputc('\n', sc->fout);
	}
}

