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


ASSUME_NONNULL_BEGIN

/* Create a template with placeholders given a format string */
RETURNS_OWNED Template* Template_create(const char* fmt);

/* Destructor */
void Template_free(CONSUMED Template* _Nullable tp);

/* Fill in placeholders but do not evaluate result */
RETURNS_OWNED Value* Template_fill(const Template* tp, OWNED ...);
RETURNS_OWNED Value* Template_fillv(const Template* tp, OWNED va_list args);
RETURNS_OWNED Value* Template_staticFill(Template* _Nullable * _Nonnull ptp, const char* fmt, OWNED ...);

/* Fill in placeholders and evaluate result */
RETURNS_OWNED Value* Template_eval(const Template* tp, const Context* ctx, OWNED ...);
RETURNS_OWNED Value* Template_evalv(const Template* tp, const Context* ctx, OWNED va_list args);
RETURNS_OWNED Value* Template_staticEval(Template* _Nullable * _Nonnull ptp, const Context* ctx, const char* fmt, OWNED ...);

/* Number of placeholders that must be filled */
unsigned Template_placeholderCount(const Template* tp);

ASSUME_NONNULL_END

#endif /* SC_TEMPLATE_H */
