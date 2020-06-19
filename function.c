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
		free(func->argnames[i]);
	}
	free(func->argnames);
	
	Value_free(func->body);
	
	free(func);
}

Function* Function_copy(const Function* func) {
	if(!func) {
		return NULL;
	}
	
	char** argsCopy = fmalloc(func->argcount * sizeof(*argsCopy));
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		argsCopy[i] = strdup(func->argnames[i]);
	}
	
	return Function_new(func->argcount, argsCopy, Value_copy(func->body));
}

Value* Function_eval(const Function* func, const Context* ctx, const ArgList* arglist) {
	if(func->argcount != arglist->count) {
		return ValErr(typeError("Function expects %u argument%s, not %u.", func->argcount, func->argcount == 1 ? "" : "s", arglist->count));
	}
	
	Context* frame = Context_pushFrame(ctx);
	
	unsigned i;
	for(i = 0; i < arglist->count; i++) {
		Value* argval = Value_copy(arglist->args[i]);
		
		/*
		 * When variables in this expression are evaluated, they need to
		 * be looked up in the correct scope (above this function's frame).
		 * This is to allow for lazy evaluation of parameter expressions.
		 */
		Value_setScope(argval, ctx);
		Context_addLocal(frame, strdup(func->argnames[i]), argval);
	}
	
	Value* ret = Value_eval(func->body, frame);
	
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
			free(ret);
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
		free(args);
		*err = badChar(*expr);
		return NULL;
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
					free(args[i]);
				}
				free(args);
				
				*err = badChar(*expr);
				return NULL;
			}
			
			trimSpaces(expr);
		}
	}
	
	if(arg) {
		free(arg);
	}
	
	if(**expr != end) {
		/* Invalid character inside argument name list */
		if(args) {
			/* Free argument names and return */
			unsigned i;
			for(i = 0; i < len; i++) {
				free(args[i]);
			}
			free(args);
		}
		
		*err = badChar(*expr);
		return NULL;
	}
	
	/* Skip closing parenthesis */
	(*expr)++;
	
	return Function_new(len, args, NULL);
}

char* Function_repr(const Function* func, const char* name, bool pretty) {
	char* ret;
	char* args = argsToString(func);
	char* body = Value_repr(func->body, pretty, false);
	
	if(name != NULL) {
		asprintf(&ret, "%s(%s) = %s", name, args ?: "", body);
	}
	else {
		asprintf(&ret, "|%s| %s", args ?: "", body);
	}
	
	free(body);
	free(args);
	return ret;
}

char* Function_wrap(const Function* func, const char* name, bool top) {
	char* ret;
	char* args = argsToString(func);
	char* body = Value_wrap(func->body, false);
	
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
	
	free(body);
	free(args);
	return ret;
}

char* Function_verbose(const Function* func, unsigned indent) {
	char* ret;
	char* args = argsToString(func);
	char* body = Value_verbose(func->body, indent + 1);
	
	asprintf(&ret,
			 "|%s| {\n"
				 "%s%s\n"
			 "}",
			 args ?: "",
			 indentation(indent + 1), body);
	
	free(body);
	free(args);
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
			free(ret);
			ret = tmp;
		}
	}
	
	return ret;
}

char* Function_xml(const Function* func, unsigned indent) {
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
	char* body = Value_xml(func->body, indent + 2);
	
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
		
		free(args);
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
	
	free(body);
	return ret;
}

