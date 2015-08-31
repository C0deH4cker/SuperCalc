/*
  template.c
  SuperCalc

  Created by C0deH4cker on 8/27/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#include "template.h"
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "generic.h"
#include "error.h"

struct Template {
	Value* tree;
	unsigned num_placeholders;
	unsigned capacity;
	Value** placeholders;
};

/*
 Example: "@1i*4 + @1i - @2f"
 
      Value* tree;
            -          unsigned num_placeholders = 2;
          /   \        unsigned capacity = 4;
         +    @2v           0           1         2      3
        / \    |       +-----------+-----------+------+------+
       *  @1v  |       | VAL_PLACE | VAL_PLACE | NULL | NULL | <-- Value** placeholders;
      / \  |   +--+    +-----------+-----------+------+------+
    @1v  4 |      |         |           |
      |    +----------------+           |
      |    |      |                     |
      +----+      +---------------------+
      |           |
      V           V
 +-----------+------------+
 | VAL_PLACE | VAL_PLACE  |
 | (VAL_INT) | (VAL_FRAC) |
 +-----------+------------+
*/


static VALTYPE get_placeholder_type(char type) {
	switch(type) {
		case 'i': return VAL_INT;
		case 'r': return VAL_REAL;
		case 'f': return VAL_FRAC;
		case 'x': return VAL_EXPR;
		case 'u': return VAL_UNARY;
		case 'c': return VAL_CALL;
		case 'n': return VAL_VAR;
		case 'v': return VAL_VEC;
		case '@': return VAL_PLACE;
		
		default: return VAL_ERR;
	}
}

static Value* parse_placeholder(const char** expr, void* data) {
	if(**expr != '@') {
		RAISE(badChar(**expr), true);
	}
	
	Template* tp = data;
	
	/* Move past the @ character */
	(*expr)++;
	
	unsigned index = tp->num_placeholders;
	if(isdigit(**expr)) {
		/* Like @1v or @4@ */
		char* end;
		errno = 0;
		
		/* Read the number after the '@' */
		index = (unsigned)strtoul(*expr, &end, 10);
		if(errno != 0 || index == 0 || *expr == end) {
			/* An error occurred (EINVAL, ERANGE) */
			RAISE(syntaxError("Invalid number in placeholder"), true);
		}
		
		/* Because it's one-based in the format string but the array is zero-based */
		--index;
		
		/* Advance past the number */
		*expr = end;
	}
	else if(isalpha(**expr) || **expr == '_') {
		/* Like @sqrt() or @v */
		char next = (*expr)[1];
		if(isalnum(next) || next == '_') {
			/* Like @sqrt(), for internal calls. Name must be at least 2 chars long! */
			char* token = nextToken(expr);
			char* varname;
			asprintf(&varname, "@%s", token);
			free(token);
			
			Value* ret = ValVar(varname);
			free(varname);
			return ret;
		}
	}
	
	/* Check if the placeholders array needs to expand to fit the next one */
	if(index + 1 > tp->capacity) {
		/* Never expand by less than 150% */
		unsigned oldcap = tp->capacity;
		unsigned newcap = MAX(3 * oldcap / 2, index + 1);
		
		/* Zero-expand the placeholders array */
		tp->placeholders = frealloc(tp->placeholders, newcap * sizeof(*tp->placeholders));
		memset(tp->placeholders + oldcap, 0, (newcap - oldcap) * sizeof(*tp->placeholders));
		
		tp->capacity = newcap;
		tp->num_placeholders = index + 1;
	}
	
	/* Already encountered a format code with the specified index, so use same ptr */
	if(tp->placeholders[index] != NULL) {
		if(get_placeholder_type(*(*expr)++) != tp->placeholders[index]->fill) {
			RAISE(typeError("Type mismatch of numbered placeholders."), true);
		}
		
		return tp->placeholders[index];
	}
	
	/* Create a new placeholder at the specified index */
	return tp->placeholders[index] = ValPlace(get_placeholder_type(*(*expr)++));
}


Template* Template_create(const char* fmt) {
	Template* ret = fcalloc(1, sizeof(*ret));
	
	/* Like normal parsing but handle '@' specially by building placeholders */
	parser_cb cb = {&parse_placeholder, ret};
	ret->tree = Value_parse(&fmt, 0, 0, &cb);
	
	if(ret->capacity > ret->num_placeholders) {
		ret->capacity = ret->num_placeholders;
		ret->placeholders = frealloc(ret->placeholders,
		                    ret->capacity * sizeof(*ret->placeholders));
	}
	
	return ret;
}

void Template_free(Template* tp) {
	Value_free(tp->tree);
	
	/* Call release on these */
	free(tp->placeholders);
	free(tp);
}

Value* Template_fill(const Template* tp, ...) {
	va_list args;
	va_start(args, tp);
	
	Value* ret = Template_fillv(tp, args);
	
	va_end(args);
	return ret;
}

static Value* next_value(VALTYPE type, va_list args) {
	switch(type) {
		case VAL_INT:   return ValInt(va_arg(args, int));
		case VAL_REAL:  return ValReal(va_arg(args, double));
		case VAL_FRAC:  return ValFrac(va_arg(args, Fraction*));
		case VAL_EXPR:  return ValExpr(va_arg(args, BinOp*));
		case VAL_UNARY: return ValUnary(va_arg(args, UnOp*));
		case VAL_CALL:  return ValCall(va_arg(args, FuncCall*));
		case VAL_VAR:   return ValVar(strdup(va_arg(args, const char*)));
		case VAL_VEC:   return ValVec(va_arg(args, Vector*));
		case VAL_PLACE: return va_arg(args, Value*);
			
		default:
			return ValErr(typeError("Unexpected placeholder type", type));
	}
}

Value* Template_fillv(const Template* tp, va_list args) {
	Value* ret = NULL;
	
	/* Backup placeholder types array */
	VALTYPE* orig_types = fmalloc(tp->num_placeholders * sizeof(*orig_types));
	
	/* Fill in placholders */
	unsigned i;
	for(i = 0; i < tp->num_placeholders; i++) {
		Value* cur = tp->placeholders[i];
		if(cur == NULL) {
			ret = ValErr(missingPlaceholder(i));
			break;
		}
		
		if(cur->type != VAL_PLACE) {
			badValType(cur->type);
		}
		
		/* Replace the placeholder with the value of the argument */
		orig_types[i] = cur->fill;
		Value* arg = next_value(cur->fill, args);
		memcpy(cur, arg, sizeof(*cur));
		free(arg);
	}
	
	/* Only copy tree when there's no error */
	if(ret == NULL) {
		ret = Value_copy(tp->tree);
	}
	
	/* Undo placeholder replacement */
	for(i = 0; i < tp->num_placeholders; i++) {
		Value* cur = tp->placeholders[i];
		if(cur->type != VAL_PLACE) {
			/* Hack. Allocate a new value just to delete its contents */
			Value* onDeathRow = fmalloc(sizeof(*onDeathRow));
			memcpy(onDeathRow, cur, sizeof(*onDeathRow));
			Value_free(onDeathRow);
			
			cur->type = VAL_PLACE;
			cur->fill = orig_types[i];
		}
	}
	
	return ret;
}

Value* Template_eval(const Template* tp, const Context* ctx, ...) {
	va_list args;
	va_start(args, ctx);
	
	Value* ret = Template_evalv(tp, ctx, args);
	
	va_end(args);
	return ret;
}

Value* Template_evalv(const Template* tp, const Context* ctx, va_list args) {
	Value* filled = Template_fillv(tp, args);
	Value* ret = Value_eval(filled, ctx);
	Value_free(filled);
	return ret;
}

char Template_placeholderChar(VALTYPE type) {
	switch(type) {
		case VAL_INT:    return 'i';
		case VAL_REAL:   return 'r';
		case VAL_FRAC:   return 'f';
		case VAL_EXPR:   return 'x';
		case VAL_UNARY:  return 'u';
		case VAL_CALL:   return 'c';
		case VAL_VAR:    return 'n';
		case VAL_VEC:    return 'v';
		case VAL_PLACE:  return '@';
			
		default:         return '\0';
	}
}

unsigned Template_placeholderCount(const Template* tp) {
	return tp->num_placeholders;
}
