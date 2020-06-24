/*
  func.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "function.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "support.h"
#include "error.h"
#include "generic.h"
#include "context.h"
#include "value.h"
#include "arglist.h"
#include "variable.h"


static char* argsToString(const Function* func);


Function* Function_new(unsigned argcount, char** argnames, Value* body) {
	Function* ret = fmalloc(sizeof(*ret));
	
	ret->argcount = argcount;
	ret->argnames = argnames;
	ret->body = body;
	
	return ret;
}

void Function_free(Function* func) {
	if(!func) {
		return;
	}
	
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		destroy(func->argnames[i]);
	}
	destroy(func->argnames);
	
	Value_free(func->body);
	
	destroy(func);
}

Function* Function_copy(const Function* func) {
	char** argsCopy = fmalloc(func->argcount * sizeof(*argsCopy));
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		argsCopy[i] = strdup(func->argnames[i]);
	}
	
	Value* bodyCopy = func->body;
	if(bodyCopy != NULL) {
		bodyCopy = Value_copy(bodyCopy);
	}
	return Function_new(func->argcount, argsCopy, bodyCopy);
}

Value* Function_eval(const Function* func, const Context* ctx, const ArgList* arglist) {
	assert(func->body != NULL);
	
	if(func->argcount != arglist->count) {
		return ValErr(typeError("Function expects %u argument%s, not %u.", func->argcount, func->argcount == 1 ? "" : "s", arglist->count));
	}
	
	Error* err = NULL;
	ArgList* evaluated = ArgList_eval(arglist, ctx, &err);
	if(evaluated == NULL) {
		return ValErr(err);
	}
	
	Context* frame = Context_pushFrame(ctx);
	
	unsigned i;
	for(i = 0; i < evaluated->count; i++) {
		Value* val = Value_copy(evaluated->args[i]);
		Variable* arg;
		char* argname = strdup(func->argnames[i]);
		
		if(val->type == VAL_VAR) {
			Variable* var = Variable_getAbove(frame, val->name);
			Value_free(val);
			arg = Variable_new(argname, Value_copy(var->val));
		}
		else {
			arg = Variable_new(argname, val);
		}
		
		Context_addLocal(frame, arg);
	}
	
	ArgList_free(evaluated);
	
	/* Asserted to be nonnull above */
	Value* ret = Value_eval(CAST_NONNULL(func->body), frame);
	
	Context_popFrame(frame);
	
	return ret;
}

static char* argsToString(const Function* func) {
	char* ret = NULL;
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		if(i == 0) {
			asprintf(&ret, "%s", func->argnames[i]);
		}
		else {
			char* tmp;
			asprintf(&tmp, "%s, %s", ret, func->argnames[i]);
			destroy(ret);
			ret = tmp;
		}
	}
	
	return ret;
}

Function* Function_parseArgs(const char** expr, char sep, char end, Error** err) {
	*err = NULL;
	
	/* Array of argument names */
	unsigned size = 2;
	char** args = fmalloc(size * sizeof(*args));
	unsigned len = 0;
	
	/* Add each argument name to the array */
	char* arg = nextToken(expr);
	
	if(arg == NULL && **expr != end) {
		/* Invalid character */
		destroy(args);
		*err = badChar(*expr);
		return NULL;
	}
	
	trimSpaces(expr);
	
	if(arg == NULL) {
		/* Empty parameter list means function with no args */
		destroy(args);
		args = NULL;
		len = 0;
	}
	else {
		/* Loop through each argument in the list */
		while(**expr == sep || **expr == end) {
			args[len++] = arg;
			arg = NULL;
			
			if(**expr == end) {
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
				/* Free argument names and return */
				unsigned i;
				for(i = 0; i < len; i++) {
					destroy(args[i]);
				}
				destroy(args);
				
				*err = badChar(*expr);
				return NULL;
			}
			
			trimSpaces(expr);
		}
	}
	
	if(arg) {
		destroy(arg);
	}
	
	if(**expr != end) {
		/* Invalid character inside argument name list */
		if(args) {
			/* Free argument names and return */
			unsigned i;
			for(i = 0; i < len; i++) {
				destroy(args[i]);
			}
			destroy(args);
		}
		
		*err = badChar(*expr);
		return NULL;
	}
	
	/* Skip closing parenthesis */
	(*expr)++;
	
	return Function_new(len, args, NULL);
}

char* Function_repr(const Function* func, const char* name, bool pretty) {
	assert(func->body != NULL);
	
	char* ret;
	char* args = argsToString(func);
	char* body = Value_repr(CAST_NONNULL(func->body), pretty, false);
	
	if(name != NULL) {
		asprintf(&ret, "%s(%s) = %s", name, args ?: "", body);
	}
	else {
		asprintf(&ret, "|%s| %s", args ?: "", body);
	}
	
	destroy(body);
	destroy(args);
	return ret;
}

char* Function_wrap(const Function* func, const char* name, bool top) {
	assert(func->body != NULL);
	
	char* ret;
	char* args = argsToString(func);
	char* body = Value_wrap(CAST_NONNULL(func->body), false);
	
	if(name != NULL) {
		asprintf(&ret, "%s(%s) = %s", name, args ?: "", body);
	}
	else {
		const char* format;
		if(top) {
			format = "|%s| %s";
		}
		else {
			format = "(|%s| %s)";
		}
		
		asprintf(&ret, format, args ?: "", body);
	}
	
	destroy(body);
	destroy(args);
	return ret;
}

char* Function_verbose(const Function* func, unsigned indent) {
	assert(func->body != NULL);
	
	char* ret;
	char* args = argsToString(func);
	char* body = Value_verbose(CAST_NONNULL(func->body), indent + 1);
	
	asprintf(&ret,
			 "|%s| {\n"
				 "%s%s\n"
			 "}",
			 args ?: "",
			 indentation(indent + 1), body);
	
	destroy(body);
	destroy(args);
	return ret;
}

static char* argsXml(const Function* func, unsigned indent) {
	char* ret = NULL;
	
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		if(i == 0) {
			asprintf(&ret, "%s<arg name=\"%s\"/>",
					 indentation(indent), func->argnames[i]);
		}
		else {
			char* tmp;
			asprintf(&tmp,
					 "%s\n"
					 "%s<arg name=\"%s\"/>",
					 ret,
					 indentation(indent), func->argnames[i]);
			destroy(ret);
			ret = tmp;
		}
	}
	
	return ret;
}

char* Function_xml(const Function* func, unsigned indent) {
	assert(func->body != NULL);
	
	/*
	 sc> ?x f(x) = 3x + 4
	 
	 <vardata name="f">
	   <func>
	     <argnames>
	       <arg name="x"/>
	     </argnames>
	     <expr>
	       <add>
	         <mul>
	           <int>3</int>
	           <var name="x"/>
	         </mul>
	         <int>4</int>
	       </add>
	     </expr>
	   </func>
	 </vardata>
	*/
	char* ret;
	char* body = Value_xml(CAST_NONNULL(func->body), indent + 2);
	
	if(func->argcount > 0) {
		char* args = argsXml(func, indent + 2);
		
		asprintf(&ret,
				 "<func>\n"
					 "%2$s<argnames>\n"
						 "%4$s\n" /* args */
					 "%2$s</argnames>\n"
					 "%2$s<expr>\n"
						 "%3$s%5$s\n" /* body */
					 "%2$s</expr>\n"
				 "%1$s</func>",
				 indentation(indent), indentation(indent + 1), indentation(indent + 2),
				 args,
				 body);
		
		destroy(args);
	}
	else {
		asprintf(&ret,
				 "<func>\n"
					 "%2$s<argnames/>\n"
					 "%2$s<expr>\n"
						 "%3$s%4$s\n" /* body */
					 "%2$s</expr>\n"
				 "%1$s</func>",
				 indentation(indent), indentation(indent + 1), indentation(indent + 2),
				 body);
	}
	
	destroy(body);
	return ret;
}

