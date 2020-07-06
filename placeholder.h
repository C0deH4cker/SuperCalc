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
#include "generic.h"


ASSUME_NONNULL_BEGIN

typedef enum {
	PH_ERR = -1,
	PH_UNINITIALIZED = 0,
	PH_INT,
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


DECL(Placeholder);

/* Constructors */
RETURNS_OWNED Placeholder* Placeholder_new(PLACETYPE type, unsigned index);
RETURNS_OWNED Placeholder* Placeholder_create(char type, unsigned index);
RETURNS_OWNED Placeholder* Placeholder_fromString(const char* fmt);

/* Destructor */
void Placeholder_free(CONSUMED Placeholder* ph);

/* Copying */
RETURNS_OWNED Placeholder* Placeholder_copy(const Placeholder* ph);

/* Parse a placeholder from a format string */
RETURNS_OWNED Placeholder* Placeholder_parse(INOUT istring expr);

/* Printing */
RETURNS_OWNED char* Placeholder_repr(const Placeholder* ph);
RETURNS_OWNED char* Placeholder_xml(const Placeholder* ph, unsigned indent);

METHOD_debugString(Placeholder);

ASSUME_NONNULL_END

#endif /* SC_PLACEHOLDER_H */
