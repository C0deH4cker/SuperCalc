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
Context* Context_copy(Context* ctx);

/* Variable accessing */
/* These methods consume the `var` argument. */
void Context_addGlobal(Context* ctx, Variable* var);
void Context_addLocal(Context* ctx, Variable* var);
void Context_setGlobal(Context* ctx, const char* name, Variable* var);
void Context_setLocal(Context* ctx, const char* name, Variable* var);

/* Stack frames */
void Context_pushLocals(Context* ctx);
void Context_popLocals(Context* ctx);

/* Variable deletion */
void Context_del(Context* ctx, const char* name);

/*
 Context_get returns a pointer from within the context list, so do
 not free the returned variable.
*/
Variable* Context_get(Context* ctx, const char* name);

#endif
