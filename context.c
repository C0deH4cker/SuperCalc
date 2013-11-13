/*
  context.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "context.h"
#include <stdlib.h>
#include <string.h>

#include "generic.h"
#include "variable.h"


struct VarList {
	Variable* var;
	struct VarList* next;
};

struct Context {
	struct VarList* varlist;
};

Context* Context_new(void) {
	Context* ret = fmalloc(sizeof(*ret));
	
	ret->varlist = NULL;
	
	Context_add(ret, VarValue("ans", ValInt(0)));
	
	return ret;
}

void Context_free(Context* ctx) {
	struct VarList* cur = ctx->varlist;
	if(cur == NULL) {
		free(ctx);
		return;
	}
	
	struct VarList* next = cur->next;
	
	while(cur) {
		/* Free current element */
		Variable_free(cur->var);
		free(cur);
		
		/* Go to next element in the linked list */
		cur = next;
		if(cur)
			next = cur->next;
	}
	
	free(ctx);
}

Context* Context_copy(Context* ctx) {
	if(!ctx) return NULL;
	
	Context* ret = Context_new();
	
	/* Try to keep the elements in the same order */
	struct VarList* src = ctx->varlist;
	
	struct VarList* dst = ret->varlist;
	struct VarList* prev = NULL;
	
	while(src) {
		/* Allocate next node */
		dst = fmalloc(sizeof(*dst));
		dst->var = Variable_copy(src->var);
		dst->next = NULL;
		
		
		if(prev == NULL) {
			/* Link new node to list root */
			ret->varlist = dst;
		}
		else {
			/* Link previous node to newly created one */
			prev->next = dst;
		}
		
		/* Save pointer to current node as the previous one for next loop iteration */
		prev = dst;
		
		/* Move to next source node */
		src = src->next;
	}
	
	return ret;
}

void Context_add(Context* ctx, Variable* var) {
	struct VarList* elem = fmalloc(sizeof(*elem));
	
	elem->var = var;
	
	/* Put the new element in the front of the linked list and move the rest back */
	elem->next = ctx->varlist;
	ctx->varlist = elem;
}

Variable* Context_get(Context* ctx, const char* name) {
	struct VarList* cur = ctx->varlist;
	
	while(cur) {
		if(strcmp(cur->var->name, name) == 0) {
			return cur->var;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

void Context_set(Context* ctx, const char* name, Variable* var) {
	if(var->type == VAR_FUNC && strcmp(name, "ans") == 0) {
		RAISE(nameError("Cannot redefine special varaible 'ans' as a function."));
		return;
	}
	
	Variable* cpy = Variable_copy(var);
	Variable* dst = Context_get(ctx, name);
	
	if(dst == NULL) {
		/* Variable doesn't yet exist, so create it. */
		if(cpy->name != NULL)
			free(cpy->name);
		
		cpy->name = strdup(name);
		
		Context_add(ctx, cpy);
	}
	else {
		if(dst->type == VAR_BUILTIN) {
			RAISE(typeError("Unable to modify builtin variable '%s'.", dst->name));
			return;
		}
		
		/* Variable already exists, so update it */
		Variable_update(dst, cpy);
	}
}

void Context_del(Context* ctx, const char* name) {
	struct VarList* cur = ctx->varlist;
	struct VarList* prev = NULL;
	
	if(strcmp(name, "ans") == 0) {
		RAISE(nameError("Cannot delete special variable 'ans'."));
		return;
	}
	
	while(cur) {
		if(strcmp(cur->var->name, name) == 0) {
			if(cur->var->type == VAR_BUILTIN) {
				RAISE(typeError("Cannot delete builtin variable '%s'.", name));
				return;
			}
			
			/* Skip current element altogether */
			if(prev == NULL) {
				/* First element in linked list */
				ctx->varlist = cur->next;
			}
			else {
				prev->next = cur->next;
			}
			
			/* Free current element */
			Variable_free(cur->var);
			free(cur);
			
			return;
		}
		
		prev = cur;
		cur = cur->next;
	}
	
	RAISE(varNotFound(name));
}



