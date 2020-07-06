/*
  supercalc.h
  SuperCalc

  Created by C0deH4cker on 11/22/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_SUPERCALC_H
#define SC_SUPERCALC_H

#include <stdio.h>
#include <stdbool.h>

typedef struct SuperCalc SuperCalc;
#include "value.h"
#include "context.h"
#include "generic.h"


ASSUME_NONNULL_BEGIN

struct SuperCalc {
	OWNED Context* ctx;
	bool interactive;
	uint8_t importDepth;
};


DECL(SuperCalc);

RETURNS_OWNED SuperCalc* SuperCalc_new(void);
void SuperCalc_free(CONSUMED SuperCalc* _Nullable sc);
void SuperCalc_run(UNOWNED SuperCalc* sc);
RETURNS_OWNED Error* SuperCalc_importFile(UNOWNED SuperCalc* sc, const char* filename);
RETURNS_OWNED Value* _Nullable SuperCalc_runLine(UNOWNED SuperCalc* sc, UNOWNED char* str, VERBOSITY v);

METHOD_debugString(SuperCalc);

ASSUME_NONNULL_END

#endif /* SC_SUPERCALC_H */
