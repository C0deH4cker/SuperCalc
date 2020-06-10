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

/* Create a template with placeholders given a format string */
Template* Template_create(const char* fmt);

/* Destructor */
void Template_free(Template* tp);

/* Fill in placeholders but do not evaluate result */
Value* Template_fill(const Template* tp, ...);
Value* Template_fillv(const Template* tp, va_list args);
Value* Template_staticFill(Template** ptp, const char* fmt, ...);

/* Fill in placeholders and evaluate result */
Value* Template_eval(const Template* tp, const Context* ctx, ...);
Value* Template_evalv(const Template* tp, const Context* ctx, va_list args);
Value* Template_staticEval(Template** ptp, const Context* ctx, const char* fmt, ...);

/* Number of placeholders that must be filled */
unsigned Template_placeholderCount(const Template* tp);

#endif /* SC_TEMPLATE_H */
