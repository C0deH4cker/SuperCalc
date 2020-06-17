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

struct SuperCalc {
	OWNED NONNULL Context* ctx;
	bool interactive;
	uint8_t importDepth;
};

OWNED NONNULL SuperCalc* SuperCalc_new(void);
void SuperCalc_free(OWNED NULLABLE SuperCalc* sc);
void SuperCalc_run(UNOWNED NONNULL SuperCalc* sc);
OWNED NONNULL Error* SuperCalc_importFile(UNOWNED NONNULL SuperCalc* sc, NONNULL const char* filename);
OWNED NULLABLE Value* SuperCalc_runLine(UNOWNED NONNULL SuperCalc* sc, UNOWNED NONNULL char* str, VERBOSITY v);

#endif /* SC_SUPERCALC_H */
