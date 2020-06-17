/*
  template.h
  SuperCalc

  Created by C0deH4cker on 8/27/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#ifndef SC_TEMPLATE_H
#define SC_TEMPLATE_H

#include <stdarg.h>

#define TP(name) \
static Template* name = NULL

#define TP_FILL(name, fmt, ...) \
Template_staticFill(&name, fmt, __VA_ARGS__)

#define TP_EVAL(name, ctx, fmt, ...) \
Template_staticEval(&name, ctx, fmt, __VA_ARGS__)

typedef struct Template Template;
#include "value.h"
#include "context.h"
#include "generic.h"

/* Create a template with placeholders given a format string */
OWNED NONNULL Template* Template_create(NONNULL const char* fmt);

/* Destructor */
void Template_free(OWNED NULLABLE Template* tp);

/* Fill in placeholders but do not evaluate result */
OWNED NONNULL Value* Template_fill(NONNULL const Template* tp, OWNED ...);
OWNED NONNULL Value* Template_fillv(NONNULL const Template* tp, OWNED va_list args);
OWNED NONNULL Value* Template_staticFill(NULLABLE Template* NONNULL* ptp, NONNULL const char* fmt, OWNED ...);

/* Fill in placeholders and evaluate result */
OWNED NONNULL Value* Template_eval(NONNULL const Template* tp, NONNULL const Context* ctx, OWNED ...);
OWNED NONNULL Value* Template_evalv(NONNULL const Template* tp, NONNULL const Context* ctx, OWNED va_list args);
OWNED NONNULL Value* Template_staticEval(NULLABLE Template* NONNULL* ptp, NONNULL const Context* ctx, NONNULL const char* fmt, OWNED ...);

/* Number of placeholders that must be filled */
unsigned Template_placeholderCount(NONNULL const Template* tp);

#endif /* SC_TEMPLATE_H */
