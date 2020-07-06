/*
  context.h
  SuperCalc

  Created by C0deH4cker on 11/7/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_CONTEXT_H
#define SC_CONTEXT_H


typedef struct Context Context;
#include "variable.h"
#include "generic.h"
#include "value.h"


ASSUME_NONNULL_BEGIN

DECL(Context);

/* Constructor */
RETURNS_OWNED Context* Context_new(void);

/* Destructor */
void Context_free(CONSUMED Context* _Nullable ctx);

/* Copying */
RETURNS_OWNED Context* Context_copy(const Context* ctx);

/* Variable accessing */
void Context_addGlobal(const Context* ctx, CONSUMED Variable* var);
void Context_addLocal(const Context* ctx, CONSUMED Variable* var);
void Context_setGlobal(const Context* ctx, const char* name, CONSUMED Value* val);

/* Stack frames */
RETURNS_OWNED Context* Context_pushFrame(const Context* ctx);
void Context_popFrame(CONSUMED Context* ctx);

/* Variable deletion */
void Context_del(const Context* ctx, const char* name);
void Context_clear(UNOWNED Context* ctx);

/*
 Context_get and Context_getAbove return a pointer from within the
 context, so do not free the returned variable.
*/
RETURNS_UNOWNED Variable* _Nullable Context_get(const Context* ctx, const char* name);
RETURNS_UNOWNED Variable* _Nullable Context_getAbove(const Context* ctx, const char* name);

METHOD_debugString(Context);

ASSUME_NONNULL_END

#endif /* SC_CONTEXT_H */
