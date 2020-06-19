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
	OWNED NONNULL char* name;
	OWNED NONNULL Value* val;
	UNOWNED NULLABLE struct VarNode* next;
};

struct ContextStack {
	struct VarNode* vars;
	struct ContextStack* next;
};

struct Context {
	struct VarNode* globals;
	struct ContextStack* locals;
};


static struct VarNode* VarNode_new(OWNED NONNULL char* name, OWNED NONNULL Value* val);
static void VarNode_free(IN OWNED NULLABLE struct VarNode* node);
static void freeVars(struct VarNode* vars);
static void freeStack(struct ContextStack* stack);
static struct VarNode* copyVars(const struct VarNode* src);
static struct ContextStack* copyStack(const struct ContextStack* stack);
static void addVar(struct VarNode** vars, char* name, Value* val);
static struct VarNode* findPrev(struct VarNode* cur, const char* name);
static bool isFirst(struct VarNode* cur, const char* name);
static struct VarNode* findNode(struct VarNode* cur, const char* name);
static Value** findVar(struct VarNode* cur, const char* name);


static struct VarNode* VarNode_new(char* name, Value* val) {
	struct VarNode* ret = fmalloc(sizeof(*ret));
	ret->name = name;
	ret->val = val;
	ret->next = NULL;
	return ret;
}

static void VarNode_free(struct VarNode* node) {
	if(!node) {
		return;
	}
	
	free(node->name);
	Value_free(node->val);
	free(node);
}


Context* Context_new(void) {
	Context* ret = fmalloc(sizeof(*ret));
	
	ret->globals = VarNode_new(strdup("ans"), ValInt(0));
	ret->locals = NULL;
	
	return ret;
}

static void freeVars(struct VarNode* cur) {
	if(cur == NULL) return;
	
	struct VarNode* next = cur->next;
	
	while(cur) {
		VarNode_free(cur);
		
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
	free(ctx);
}

static struct VarNode* copyVars(const struct VarNode* src) {
	struct VarNode* ret = NULL;
	struct VarNode* cur;
	struct VarNode* prev = NULL;
	
	while(src) {
		/* Allocate next node */
		cur = VarNode_new(src->name ? strdup(src->name) : NULL, Value_copy(src->val));
		
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

static void addVar(struct VarNode** vars, char* name, Value* val) {
	struct VarNode* elem = fmalloc(sizeof(*elem));
	
	elem->name = name;
	elem->val = val;
	
	/* Put the new element in the front of the linked list and move the rest back */
	elem->next = *vars;
	*vars = elem;
}

void Context_addGlobal(const Context* ctx, char* name, Value* val) {
	/* Always keep "ans" first */
	addVar(&ctx->globals->next, name, val);
}

void Context_addLocal(const Context* ctx, char* name, Value* val) {
	if(ctx->locals == NULL) {
		DIE("Tried to add a local variable with no stack frame setup!");
	}
	
	addVar(&ctx->locals->vars, name, val);
}

void Context_setGlobal(const Context* ctx, const char* name, Value* val) {
	Value** dst = findVar(ctx->globals, name);
	if(dst == NULL) {
		/* Variable doesn't yet exist, so create it. */
		Context_addGlobal(ctx, strdup(name), val);
	}
	else {
		/* Variable already exists, so update it */
		*dst = val;
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
		if(strcmp(cur->next->name, name) == 0) {
			return cur;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

static bool isFirst(struct VarNode* cur, const char* name) {
	return cur && strcmp(cur->name, name) == 0;
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
			
			VarNode_free(cur);
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
	VarNode_free(cur);
}

void Context_clear(Context* ctx) {
	/* Delete all global variables, skipping "ans" */
	struct VarNode* prev = ctx->globals;
	struct VarNode* cur = prev->next;
	while(cur != NULL) {
		prev->next = cur->next;
		VarNode_free(cur);
		cur = prev->next;
	}
	
	/* Set ans to 0 */
	Context_setGlobal(ctx, "ans", ValInt(0));
}

static struct VarNode* findNode(struct VarNode* cur, const char* name) {
	while(cur) {
		if(strcmp(cur->name, name) == 0) {
			return cur;
		}
		
		cur = cur->next;
	}
	
	return NULL;
}

static Value** findVar(struct VarNode* cur, const char* name) {
	struct VarNode* node = findNode(cur, name);
	
	return node ? &node->val : NULL;
}

Value* Context_get(const Context* ctx, const char* name) {
	Value** pval = NULL;
	
	if(ctx->locals != NULL) {
		/* Search the top locals stack frame for the variable */
		pval = findVar(ctx->locals->vars, name);
	}
	
	/* Search globals as a last resort only if it wasn't found in locals */
	if(pval == NULL) {
		pval = findVar(ctx->globals, name);
	}
	
	return pval ? *pval : NULL;
}

Value* Context_getAbove(const Context* ctx, const char* name) {
	Value** pval;
	
	if(ctx->locals != NULL) {
		/* Skip the current frame and walk up the call stack */
		struct ContextStack* frame;
		for(frame = ctx->locals->next; frame != NULL; frame = frame->next) {
			pval = findVar(frame->vars, name);
			if(pval != NULL) {
				return *pval;
			}
		}
	}
	
	/* Last resort, try to find a global with this name */
	pval = findVar(ctx->globals, name);
	return pval ? *pval : NULL;
}

