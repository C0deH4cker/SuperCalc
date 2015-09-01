/*
  context.c
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "context.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "generic.h"
#include "variable.h"


struct VarNode {
	Variable* var;
	struct VarNode* next;
};

struct ContextStack {
	struct VarNode* vars;
	struct ContextStack* next;
};

struct Context {
	struct VarNode* globals;
	struct ContextStack* locals;
};


static void freeVars(struct VarNode* vars);
static void freeStack(struct ContextStack* stack);
static struct VarNode* copyVars(const struct VarNode* src);
static struct ContextStack* copyStack(const struct ContextStack* stack);
static void addVar(struct VarNode** vars, Variable* var);
static struct VarNode* findPrev(struct VarNode* cur, const char* name);
static bool isFirst(struct VarNode* cur, const char* name);
static struct VarNode* findNode(struct VarNode* cur, const char* name);
static Variable* findVar(struct VarNode* cur, const char* name);


Context* Context_new(void) {
	Context* ret = fmalloc(sizeof(*ret));
	
	ret->globals = fmalloc(sizeof(*ret->globals));
	ret->globals->var = VarValue(strdup("ans"), ValInt(0));
	ret->globals->next = NULL;
	ret->locals = NULL;
	
	return ret;
}

static void freeVars(struct VarNode* cur) {
	if(cur == NULL) return;
	
	struct VarNode* next = cur->next;
	
	while(cur) {
		/* Free current element */
		Variable_free(cur->var);
		free(cur);
		
		/* Go to next element in the linked list */
		cur = next;
		if(cur) {
			next = cur->next;
		}
	}
}

static void freeStack(struct ContextStack* stack) {
	while(stack) {
		struct ContextStack* next = stack->next;
		freeVars(stack->vars);
		stack = next;
	}
}

void Context_free(Context* ctx) {
	freeVars(ctx->globals);
	freeStack(ctx->locals);
	
	free(ctx);
}

static struct VarNode* copyVars(const struct VarNode* src) {
	struct VarNode* ret = NULL;
	struct VarNode* cur;
	struct VarNode* prev = NULL;
	
	while(src) {
		/* Allocate next node */
		cur = fmalloc(sizeof(*cur));
		cur->var = Variable_copy(src->var);
		cur->next = NULL;
		
		
		if(prev) {
			/* Link previous node to newly created one */
			prev->next = cur;
		}
		else {
			/* List root */
			ret = cur;
		}
		
		/* Save pointer to current node as the previous one for next loop iteration */
		prev = cur;
		
		/* Move to next source node */
		src = src->next;
	}
	
	return ret;
}

static struct ContextStack* copyStack(const struct ContextStack* stack) {
	struct ContextStack* ret = NULL;
	struct ContextStack* prev = NULL;
	
	while(stack) {
		struct ContextStack* dst = fmalloc(sizeof(*dst));
		dst->vars = copyVars(stack->vars);
		dst->next = NULL;
		
		if(ret == NULL) {
			ret = dst;
		}
		
		if(prev) {
			prev->next = dst;
		}
		
		prev = dst;
		dst = dst->next;
		stack = stack->next;
	}
	
	return ret;
}

Context* Context_copy(const Context* ctx) {
	if(!ctx) return NULL;
	
	Context* ret = Context_new();
	
	ret->globals = copyVars(ctx->globals);
	ret->locals = copyStack(ctx->locals);
	
	return ret;
}

static void addVar(struct VarNode** vars, Variable* var) {
	struct VarNode* elem = fmalloc(sizeof(*elem));
	
	elem->var = var;
	
	/* Put the new element in the front of the linked list and move the rest back */
	elem->next = *vars;
	*vars = elem;
}

void Context_addGlobal(const Context* ctx, Variable* var) {
	/* Always keep "ans" first */
	addVar(&ctx->globals->next, var);
}

void Context_addLocal(const Context* ctx, Variable* var) {
	if(ctx->locals == NULL) {
		DIE("Tried to add a local variable with no stack frame setup!");
	}
	
	addVar(&ctx->locals->vars, var);
}

void Context_setGlobal(const Context* ctx, const char* name, Variable* var) {
	if(var->type == VAR_FUNC && strcmp(name, "ans") == 0) {
		RAISE(nameError("Cannot redefine special varaible 'ans' as a function."), false);
		return;
	}
	
	Variable* dst = findVar(ctx->globals, name);
	
	if(dst == NULL) {
		/* Variable doesn't yet exist, so create it. */
		/* Make sure we are assigning the correct variable */
		if(var->name != NULL) {
			free(var->name);
		}
		
		var->name = strdup(name);
		
		Context_addGlobal(ctx, var);
	}
	else {
		if(dst->type == VAR_BUILTIN) {
			RAISE(typeError("Unable to modify builtin variable '%s'.", dst->name), false);
			return;
		}
		
		/* Variable already exists, so update it */
		Variable_update(dst, var);
	}
}

Context* Context_pushFrame(const Context* ctx) {
	Context* ret = Context_new();
	ret->globals = ctx->globals;
	
	struct ContextStack* frame = fcalloc(1, sizeof(*frame));
	
	frame->next = ctx->locals;
	ret->locals = frame;
	
	return ret;
}

void Context_popFrame(Context* ctx) {
	freeVars(ctx->locals->vars);
	free(ctx->locals);
	free(ctx);
}

static struct VarNode* findPrev(struct VarNode* cur, const char* name) {
	if(cur == NULL) return NULL;
	
	while(cur->next) {
		if(strcmp(cur->next->var->name, name) == 0) {
			return cur;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

static bool isFirst(struct VarNode* cur, const char* name) {
	return cur && strcmp(cur->var->name, name) == 0;
}

void Context_del(const Context* ctx, const char* name) {
	struct VarNode* cur;
	struct VarNode* prev = NULL;
	
	if(strcmp(name, "ans") == 0) {
		RAISE(nameError("Cannot delete special variable 'ans'."), false);
		return;
	}
	
	/* First node in locals linked list */
	if(ctx->locals != NULL) {
		if(isFirst(ctx->locals->vars, name)) {
			/* Link the previous node to the next one */
			cur = ctx->locals->vars;
			ctx->locals->vars = cur->next;
			
			/* Free node */
			Variable_free(cur->var);
			free(cur);
			
			return;
		}
		
		/* Search current locals stack frame first */
		prev = findPrev(ctx->locals->vars, name);
	}
	
	/* If prev is still NULL, search globals for it */
	prev = prev ?: findPrev(ctx->globals, name);
	
	/* If prev is STILL NULL, it wasn't found */
	if(prev == NULL) {
		RAISE(varNotFound(name), false);
		return;
	}
	
	cur = prev->next;
	
	if(cur->var->type == VAR_BUILTIN) {
		RAISE(typeError("Cannot delete builtin variable '%s'.", name), false);
		return;
	}
	
	/* Link previous node to next one */
	prev->next = cur->next;
	
	/* Free current node */
	Variable_free(cur->var);
	free(cur);
}

void Context_clear(Context* ctx) {
	/* Delete all global variables that aren't builtins */
	struct VarNode* prev = ctx->globals;
	struct VarNode* cur = prev->next;
	while(cur != NULL) {
		if(cur->var->type != VAR_BUILTIN) {
			prev->next = cur->next;
			Variable_free(cur->var);
			free(cur);
		}
		
		prev = prev->next;
		cur = prev->next;
	}
}

static struct VarNode* findNode(struct VarNode* cur, const char* name) {
	while(cur) {
		if(strcmp(cur->var->name, name) == 0) {
			return cur;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

static Variable* findVar(struct VarNode* cur, const char* name) {
	struct VarNode* node = findNode(cur, name);
	
	return node ? node->var : NULL;
}

Variable* Context_get(const Context* ctx, const char* name) {
	Variable* ret = NULL;
	
	if(ctx->locals != NULL) {
		/* Search the top locals stack frame for the variable */
		ret = findVar(ctx->locals->vars, name);
	}
	
	/* Search globals as a last resort only if it wasn't found in locals */
	return ret ?: findVar(ctx->globals, name);
}

Variable* Context_getAbove(const Context* ctx, const char* name) {
	Variable* ret = NULL;
	
	if(ctx->locals != NULL && ctx->locals->next != NULL) {
		ret = findVar(ctx->locals->next->vars, name);
	}
	
	return ret ?: findVar(ctx->globals, name);
}

