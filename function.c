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


static char* argsVerbose(const Function* func);


Function* Function_new(unsigned argcount, char** argnames, Value* body) {
	Function* ret = fmalloc(sizeof(*ret));
	
	ret->argcount = argcount;
	ret->argnames = argnames;
	ret->body = body;
	
	return ret;
}

void Function_free(Function* func) {
	unsigned i;
	for(i = 0; i < func->argcount; i++) {
		free(func->argnames[i]);
	}
	free(func->argnames);
	
	Value_free(func->body);
	
	free(func);
}

Function* Function_copy(const Function* func) {
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
	
	ArgList* evaluated = ArgList_eval(arglist, ctx);
	if(evaluated == NULL) {
		return ValErr(ignoreError());
	}
	
	Context* frame = Context_pushFrame(ctx);
	
	unsigned i;
	for(i = 0; i < evaluated->count; i++) {
		Value* val = Value_copy(evaluated->args[i]);
		Variable* arg;
		char* argname = strdup(func->argnames[i]);
		
		if(val->type == VAL_VAR) {
			Variable* var = Variable_getAbove(frame, val->name);
			
			switch(var->type) {
				case VAR_VALUE:
					arg = VarValue(argname, Value_copy(var->val));
					break;
				
				case VAR_FUNC:
					arg = VarFunc(argname, Function_copy(var->func));
					break;
				
				case VAR_BUILTIN:
					arg = VarBuiltin(argname, Builtin_copy(var->blt));
					break;
				
				default:
					badVarType(var->type);
			}
		}
		else {
			arg = VarValue(argname, Value_copy(val));
		}
		
		Context_addLocal(frame, arg);
	}
	
	ArgList_free(evaluated);
	
	Value* ret = Value_eval(func->body, frame);
	
	Context_popFrame(frame);
	
	return ret;
}

char* Function_repr(const Function* func, bool pretty) {
	char* ret;
	char* args = argsVerbose(func);
	char* body = Value_repr(func->body, pretty);
	
	asprintf(&ret, "(%s) = %s", args, body);
	
	free(body);
	free(args);
	return ret;
}

static char* argsVerbose(const Function* func) {
	char* ret;
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

char* Function_verbose(const Function* func) {
	char* ret;
	char* args = argsVerbose(func);
	char* body = Value_verbose(func->body, 1);
	
	asprintf(&ret,
			 "(%s) {\n"
				 "%s%s\n"
			 "}",
			 args,
			 indentation(1), body);
	
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
	char* args = argsXml(func, indent + 2);
	char* body = Value_xml(func->body, indent + 2);
	
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
	
	free(body);
	free(args);
	return ret;
}

