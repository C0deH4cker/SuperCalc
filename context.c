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
	struct VarNode* _Nullable next;
};

struct ContextStack {
	struct VarNode* vars;
	struct ContextStack* _Nullable next;
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


DEF(Context);

Context* Context_new(void) {
	Context* ret = OBJECT_ALLOC(Context);
	
	ret->globals = fmalloc(sizeof(*ret->globals));
	ret->globals->var = Variable_new(strdup("ans"), ValInt(0));
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
		destroy(cur);
		
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
	if(!ctx) {
		return;
	}
	
	freeVars(ctx->globals);
	freeStack(ctx->locals);
	OBJECT_FREE(Context, ctx);
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
	struct ContextStack** next = &ret;
	
	while(stack) {
		struct ContextStack* cur = fmalloc(sizeof(*next));
		cur->vars = copyVars(stack->vars);
		cur->next = NULL;
		*next = cur;
		
		next = &cur->next;
		stack = stack->next;
	}
	
	return ret;
}

Context* Context_copy(const Context* ctx) {
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

void Context_setGlobal(const Context* ctx, const char* name, Value* val) {
	Variable* dst = findVar(ctx->globals, name);
	if(dst == NULL) {
		/* Variable doesn't yet exist, so create it. */
		Variable* var = Variable_new(strdup(name), val);
		Context_addGlobal(ctx, var);
	}
	else {
		/* Variable already exists, so update it */
		Variable_update(dst, val);
	}
}

Context* Context_pushFrame(const Context* ctx) {
	Context* ret = OBJECT_ALLOC(Context);
	ret->globals = ctx->globals;
	
	struct ContextStack* frame = fcalloc(1, sizeof(*frame));
	
	frame->next = ctx->locals;
	ret->locals = frame;
	
	return ret;
}

void Context_popFrame(Context* ctx) {
	freeVars(ctx->locals->vars);
	destroy(ctx->locals);
	OBJECT_FREE(Context, ctx);
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
			destroy(cur);
			
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
	
	/* Link previous node to next one */
	prev->next = cur->next;
	
	/* Free current node */
	Variable_free(cur->var);
	destroy(cur);
}

void Context_clear(Context* ctx) {
	/* Delete all global variables, skipping "ans" */
	struct VarNode* prev = ctx->globals;
	struct VarNode* cur = prev->next;
	while(cur != NULL) {
		prev->next = cur->next;
		Variable_free(cur->var);
		destroy(cur);
		cur = prev->next;
	}
	
	/* Set ans to 0 */
	Context_setGlobal(ctx, "ans", ValInt(0));
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
	if(ctx->locals != NULL) {
		/* Skip the current frame and walk up the call stack */
		struct ContextStack* frame;
		for(frame = ctx->locals->next; frame != NULL; frame = frame->next) {
			Variable* var = findVar(frame->vars, name);
			if(var != NULL) {
				return var;
			}
		}
	}
	
	/* Last resort, try to find a global with this name */
	return findVar(ctx->globals, name);
}

RETURNS_OWNED static char* VarNode_debugString(const struct VarNode* _Nullable vars) {
	if(vars == NULL) {
		return NULL;
	}
	
	char* tail = VarNode_debugString(vars->next);
	
	char* joined = NULL;
	asprintf(&joined, "%s,%s", vars->var->name ?: "(null)", tail);
	destroy(tail);
	
	return joined;
}

RETURNS_OWNED static char* ContextStack_debugString(const struct ContextStack* _Nullable frame) {
	if(frame == NULL) {
		return NULL;
	}
	
	char* head = VarNode_debugString(frame->vars);
	char* tail = ContextStack_debugString(frame->next);
	if(!tail) {
		return head;
	}
	
	char* joined = NULL;
	asprintf(&joined, "%s\n    %s", head, tail);
	destroy(head);
	destroy(tail);
	
	return joined;
}

METHOD_debugString(Context) {
	char* ret = NULL;
	
	char* globals = VarNode_debugString(self->globals);
	char* locals = ContextStack_debugString(self->locals);
	asprintf(&ret,
		"Context {\n"
		"  globals = %s\n"
		"  locals = [\n"
		"    %s\n"
		"  ]\n"
		"}",
		globals ?: "(null)", locals ?: "(null)"
	);
	destroy(globals);
	destroy(locals);
	
	return ret;
}

