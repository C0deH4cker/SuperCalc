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


/* Constructor */
OWNED NONNULL Context* Context_new(void);

/* Destructor */
void Context_free(OWNED NULLABLE Context* ctx);

/* Copying */
OWNED NONNULL Context* Context_copy(NULLABLE const Context* ctx);

/* Variable accessing */
/* These methods consume the `var` argument. */
void Context_addGlobal(NONNULL const Context* ctx, NONNULL Variable* var);
void Context_addLocal(NONNULL const Context* ctx, NONNULL Variable* var);
void Context_setGlobal(NONNULL const Context* ctx, NONNULL const char* name, OWNED NONNULL Value* val);

/* Stack frames */
OWNED NONNULL Context* Context_pushFrame(NONNULL const Context* ctx);
void Context_popFrame(OWNED NONNULL Context* ctx);

/* Variable deletion */
void Context_del(NONNULL const Context* ctx, NONNULL const char* name);
void Context_clear(NONNULL Context* ctx);

/*
 Context_get and Context_getAbove return a pointer from within the
 context, so do not free the returned variable.
*/
UNOWNED NULLABLE Variable* Context_get(NONNULL const Context* ctx, NONNULL const char* name);
UNOWNED NULLABLE Variable* Context_getAbove(NONNULL const Context* ctx, NONNULL const char* name);

#endif /* SC_CONTEXT_H */
