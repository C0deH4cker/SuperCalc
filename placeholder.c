/*
  placeholder.c
  SuperCalc

  Created by C0deH4cker on 8/31/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#include "placeholder.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#include "support.h"
#include "generic.h"
#include "error.h"

static PLACETYPE getPlaceholderType(char type);

static PLACETYPE getPlaceholderType(char type) {
	switch(type) {
		case 'i': return PH_INT;
		case 'r': return PH_REAL;
		case 'f': return PH_FRAC;
		case 'x': return PH_EXPR;
		case 'u': return PH_UNARY;
		case 'c': return PH_CALL;
		case 'n': return PH_VAR;
		case 'v': return PH_VEC;
		case '@': return PH_VAL;
			
		default:  return PH_ERR;
	}
}


DEF(Placeholder);

Placeholder* Placeholder_new(PLACETYPE type, unsigned index) {
	Placeholder* ret = OBJECT_ALLOC(Placeholder);
	
	ret->type = type;
	ret->index = index;
	
	return ret;
}

Placeholder* Placeholder_create(char type, unsigned index) {
	return Placeholder_new(getPlaceholderType(type), index);
}

Placeholder* Placeholder_fromString(const char* fmt) {
	return Placeholder_parse(&fmt);
}

/* Destructor */
void Placeholder_free(Placeholder* ph) {
	if(!ph) {
		return;
	}
	
	OBJECT_FREE(Placeholder, ph);
}

/* Copying */
Placeholder* Placeholder_copy(const Placeholder* ph) {
	return Placeholder_new(ph->type, ph->index);
}

/* Parse a placeholder from a format string */
Placeholder* Placeholder_parse(const char** expr) {
	if(**expr != '@') {
		RAISE(badChar(*expr), true);
	}
	
	/* Move past '@' */
	(*expr)++;
	
	unsigned index = 0;
	if(isdigit(**expr)) {
		/* Like @1v or @4@ */
		char* end;
		errno = 0;
		
		/* Read the number after the '@' */
		index = (unsigned)strtoul(*expr, &end, 10);
		if(errno != 0 || index == 0 || *expr == end) {
			/* An error occurred (EINVAL, ERANGE) */
			RAISE(syntaxError(*expr, "Invalid number in placeholder"), true);
		}
		
		/* Advance past the number */
		*expr = end;
	}
	
	PLACETYPE type = getPlaceholderType(**expr);
	if(type == PH_ERR) {
		RAISE(badChar(*expr), true);
	}
	
	(*expr)++;
	
	return Placeholder_new(type, index);
}

/* Get the format character for the specified Value type */
static char getFormatChar(PLACETYPE type) {
	switch(type) {
		case PH_INT:    return 'i';
		case PH_REAL:   return 'r';
		case PH_FRAC:   return 'f';
		case PH_EXPR:   return 'x';
		case PH_UNARY:  return 'u';
		case PH_CALL:   return 'c';
		case PH_VAR:    return 'n';
		case PH_VEC:    return 'v';
		case PH_VAL:    return '@';
			
		default:         return '\0';
	}
}

/* Printing */
char* Placeholder_repr(const Placeholder* ph) {
	char* ret;
	
	if(ph->index > 0) {
		asprintf(&ret, "@%u%c", ph->index, getFormatChar(ph->type));
	}
	else {
		asprintf(&ret, "@%c", getFormatChar(ph->type));
	}
	
	return ret;
}

char* Placeholder_xml(const Placeholder* ph, unsigned indent) {
	UNREFERENCED_PARAMETER(indent);
	
	/*
	 "@1i^(1/2)"
	 <pow>
	   <placeholder type="i" index="1"/>
	   <div>
	     <int>1</int>
	     <int>2</int>
	   </div>
	 </pow>
	*/
	char* ret;
	
	if(ph->index > 0) {
		asprintf(&ret,
				 "<placeholder type=\"%c\" index=\"%u\"/>",
				 getFormatChar(ph->type), ph->index);
	}
	else {
		asprintf(&ret,
				 "<placeholder type=\"%c\"/>",
				 getFormatChar(ph->type));
	}
	
	return ret;
}

METHOD_debugString(Placeholder) {
	return Placeholder_repr(self);
}
