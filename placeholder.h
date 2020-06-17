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
OWNED NONNULL Placeholder* Placeholder_new(PLACETYPE type, unsigned index);
OWNED NONNULL Placeholder* Placeholder_create(char type, unsigned index);
OWNED NONNULL Placeholder* Placeholder_fromString(NONNULL const char* fmt);

/* Destructor */
void Placeholder_free(OWNED NULLABLE Placeholder* ph);

/* Copying */
OWNED NULLABLE_WHEN(ph == NULL) Placeholder* Placeholder_copy(NULLABLE const Placeholder* ph);

/* Parse a placeholder from a format string */
OWNED NONNULL Placeholder* Placeholder_parse(INOUT NONNULL const char* NONNULL* expr);

/* Printing */
OWNED NONNULL char* Placeholder_repr(NONNULL const Placeholder* ph);
OWNED NONNULL char* Placeholder_xml(NONNULL const Placeholder* ph, unsigned indent);

#endif /* SC_PLACEHOLDER_H */
