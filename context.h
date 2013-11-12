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
void Context_add(Context* ctx, Variable* var);
Variable* Context_get(Context* ctx, const char* name);
void Context_set(Context* ctx, const char* name, Variable* var);
void Context_del(Context* ctx, const char* name);

#endif
