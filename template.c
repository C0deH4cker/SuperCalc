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

#include "support.h"
#include "generic.h"
#include "error.h"
#include "placeholder.h"

struct Template {
	OWNED NONNULL Value* tree;
	INVARIANT(num_placeholders <= capacity) unsigned num_placeholders;
	unsigned capacity;
	OWNED NONNULL Value* OWNED NULLABLE_WHEN(capacity == 0)* placeholders;
};

/*
 Example: "@1i*4 + @1i - @2f"
 
       Value* tree;
             -          unsigned num_placeholders = 2;
           /   \        unsigned capacity = 4;
          +    @2f          0        1       2      3
         / \    |       +--------+--------+------+------+
        *  @1i  |       | Value* | Value* | NULL | NULL | <-- Value** placeholders;
       / \  |   +--+    +--------+--------+------+------+
     @1i  4 |      |        |        |
      |     +---------------+        |
      |     |      |                 |
      +-----+      +-----------------+
      |            |
      V            V
 +------------+-------------+
 | VAL_PLACE  | VAL_PLACE   |
 | (PH_INT:1) | (PH_FRAC:2) |
 +------------+-------------+
*/

static Value* parse_internalName(const char** expr) {
	if(**expr != '@') {
		RAISE(badChar(*expr), true);
	}
	
	/* Must match regex "@[a-zA-Z]{2,}" */
	if(!(isalpha((*expr)[1]) && isalpha((*expr)[2]))){
		return NULL;
	}
	
	/* Move past '@' */
	(*expr)++;
	
	/* Read name */
	char* token = nextToken(expr);
	char* varname;
	asprintf(&varname, "@%s", token);
	free(token);
	
	/* Wrap in Value object */
	Value* ret = ValVar(varname);
	return ret;
}

static Value* parse_extra(const char** expr, void* data) {
	/* Try to parse as an internal name, like @sqrt */
	Value* ret = parse_internalName(expr);
	if(ret != NULL) {
		return ret;
	}
	
	/* Must be a placeholder */
	Template* tp = data;
	Placeholder* ph = Placeholder_parse(expr);
	if(ph->type == PH_ERR) {
		RAISE(syntaxError(*expr, "Unable to parse placeholder"), true);
	}
	
	unsigned index = ph->index > 0 ? ph->index - 1 : tp->num_placeholders;
	
	if(index >= tp->num_placeholders) {
		tp->num_placeholders = index + 1;
	}
	
	/* Check if the placeholders array needs to expand to fit the next one */
	if(tp->num_placeholders > tp->capacity) {
		/* Never expand by less than 150% */
		unsigned oldcap = tp->capacity;
		unsigned newcap = MAX(3 * oldcap / 2, index + 1);
		
		/* Zero-expand the placeholders array */
		tp->placeholders = frealloc(tp->placeholders, newcap * sizeof(*tp->placeholders));
		memset(tp->placeholders + oldcap, 0, (newcap - oldcap) * sizeof(*tp->placeholders));
		
		tp->capacity = newcap;
	}
	
	/* Already encountered a format code with the specified index, so use same ptr */
	if(tp->placeholders[index] != NULL) {
		if(ph->type != tp->placeholders[index]->ph->type) {
			RAISE(typeError("Type mismatch of numbered placeholders."), true);
		}
		
		Placeholder_free(ph);
		return tp->placeholders[index];
	}
	
	/* Create a new placeholder at the specified index */
	return tp->placeholders[index] = ValPlace(ph);
}


Template* Template_create(const char* fmt) {
	Template* ret = fcalloc(1, sizeof(*ret));
	
	char* old_g_line = g_line;
	unsigned old_g_lineNumber = g_lineNumber;
	FILE* old_g_inputFile = g_inputFile;
	const char* old_g_inputFileName = g_inputFileName;
	
	/* I pinky promise not to modify fmt's contents */
	g_line = (char*)fmt;
	g_lineNumber = 1;
	g_inputFile = NULL;
	g_inputFileName = "<template>";
	
	/* Like normal parsing but handle '@' specially by building placeholders */
	parser_cb cb = {&parse_extra, ret};
	ret->tree = Value_parse(&fmt, 0, 0, &cb);
	
	g_line = old_g_line;
	g_lineNumber = old_g_lineNumber;
	g_inputFile = old_g_inputFile;
	g_inputFileName = old_g_inputFileName;
	
	if(ret->tree->type == VAL_ERR) {
		RAISE(ret->tree->err, true);
	}
	
	/*
	 * The parser callback will have created this array. Shrink it to fit
	 * just the needed slots.
	 */
	if(ret->capacity > ret->num_placeholders) {
		ret->capacity = ret->num_placeholders;
		ret->placeholders = frealloc(
			ret->placeholders,
			ret->capacity * sizeof(*ret->placeholders)
		);
	}
	
	return ret;
}

void Template_free(Template* tp) {
	if(!tp) {
		return;
	}
	
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

static Value* next_value(PLACETYPE type, va_list args) {
	switch(type) {
		case PH_INT:   return ValInt(va_arg(args, int));
		case PH_REAL:  return ValReal(va_arg(args, double));
		case PH_FRAC:  return ValFrac(va_arg(args, Fraction*));
		case PH_EXPR:  return ValExpr(va_arg(args, BinOp*));
		case PH_UNARY: return ValUnary(va_arg(args, UnOp*));
		case PH_CALL:  return ValCall(va_arg(args, FuncCall*));
		case PH_VAR:   return ValVar(va_arg(args, char*));
		case PH_VEC:   return ValVec(va_arg(args, Vector*));
		case PH_VAL:   return Value_copy(va_arg(args, Value*));
			
		default:
			return ValErr(typeError("Unexpected placeholder type", type));
	}
}

Value* Template_fillv(const Template* tp, va_list args) {
	Value* ret = NULL;
	
	/* Backup placeholder array */
	Placeholder** orig = fmalloc(tp->num_placeholders * sizeof(*orig));
	
	/* Fill in placeholders */
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
		orig[i] = cur->ph;
		Value* arg = next_value(cur->ph->type, args);
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
			cur->ph = orig[i];
		}
	}
	
	free(orig);
	return ret;
}

Value* Template_staticFill(Template** ptp, const char* fmt, ...) {
	if(*ptp == NULL) {
		*ptp = Template_create(fmt);
	}
	
	va_list args;
	va_start(args, fmt);
	
	Value* ret = Template_fillv(*ptp, args);
	
	va_end(args);
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

Value* Template_staticEval(Template** ptp, const Context* ctx, const char* fmt, ...) {
	if(*ptp == NULL) {
		*ptp = Template_create(fmt);
	}
	
	va_list args;
	va_start(args, fmt);
	
	Value* ret = Template_evalv(*ptp, ctx, args);
	
	va_end(args);
	return ret;
}

unsigned Template_placeholderCount(const Template* tp) {
	return tp->num_placeholders;
}

