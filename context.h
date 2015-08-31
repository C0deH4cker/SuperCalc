/*
  context.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_CONTEXT_H_
#define _SC_CONTEXT_H_


typedef struct Context Context;
#include "variable.h"


/* Constructor */
Context* Context_new(void);

/* Destructor */
void Context_free(Context* ctx);

/* Copying */
Context* Context_copy(const Context* ctx);

/* Variable accessing */
/* These methods consume the `var` argument. */
void Context_addGlobal(const Context* ctx, Variable* var);
void Context_addLocal(const Context* ctx, Variable* var);
void Context_setGlobal(const Context* ctx, const char* name, Variable* var);

/* Stack frames */
Context* Context_pushFrame(const Context* ctx);
void Context_popFrame(Context* ctx);

/* Variable deletion */
void Context_del(const Context* ctx, const char* name);
void Context_clear(Context* ctx);

/*
 Context_get and Context_getAbove return a pointer from within the
 context, so do not free the returned variable.
*/
Variable* Context_get(const Context* ctx, const char* name);
Variable* Context_getAbove(const Context* ctx, const char* name);

#endif
