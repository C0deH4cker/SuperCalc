/*
  placeholder.h
  SuperCalc

  Created by C0deH4cker on 8/31/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#ifndef SC_PLACEHOLDER_H
#define SC_PLACEHOLDER_H

typedef struct Placeholder Placeholder;
#include "value.h"

typedef enum {
	PH_ERR = -1,
	PH_INT = 0,
	PH_REAL,
	PH_FRAC,
	PH_EXPR,
	PH_UNARY,
	PH_CALL,
	PH_VAR,
	PH_VEC,
	PH_VAL
} PLACETYPE;

struct Placeholder {
	PLACETYPE type;
	unsigned index; /* One-based! Zero means index wasn't specified in the format string */
};

/* Constructors */
Placeholder* Placeholder_new(PLACETYPE type, unsigned index);
Placeholder* Placeholder_create(char type, unsigned index);
Placeholder* Placeholder_fromString(const char* fmt);

/* Destructor */
void Placeholder_free(Placeholder* ph);

/* Copying */
Placeholder* Placeholder_copy(const Placeholder* ph);

/* Parse a placeholder from a format string */
Placeholder* Placeholder_parse(const char** expr);

/* Printing */
char* Placeholder_repr(const Placeholder* ph);
char* Placeholder_xml(const Placeholder* ph, unsigned indent);

#endif /* SC_PLACEHOLDER_H */
