/*
  template.h
  SuperCalc

  Created by C0deH4cker on 8/27/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#ifndef _SC_TEMPLATE_H_
#define _SC_TEMPLATE_H_

#include <stdarg.h>

#define UNIQUIFY(name) CONCAT(name, __COUNTER__)
#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_(a, b) a##b

#define TP_MAKE(name, fmt) \
static Template* name = NULL; \
if(name == NULL) { \
	name = Template_create(fmt); \
}

#define TP_FILL(fmt, ...) TP_FILL_(UNIQUIFY(_fill_template_), fmt, ##__VA_ARGS__)
#define TP_FILL_(name, fmt, ...) ({ \
	TP_MAKE(name, fmt); \
	Template_fill(name, ##__VA_ARGS__); \
})

#define TP_EVAL(fmt, ...) TP_EVAL_(UNIQUIFY(_eval_template_), fmt, ##__VA_ARGS__)
#define TP_EVAL_(name, fmt, ...) ({ \
	TP_MAKE(name, fmt); \
	Template_eval(name, ##__VA_ARGS__); \
})

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

/* Fill in placeholders and evaluate result */
Value* Template_eval(const Template* tp, const Context* ctx, ...);
Value* Template_evalv(const Template* tp, const Context* ctx, va_list args);

/* Number of placeholders that must be filled */
unsigned Template_placeholderCount(const Template* tp);

#endif /* _SC_TEMPLATE_H_ */
