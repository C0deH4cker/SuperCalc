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
	Context* ctx;
	bool interactive;
};

SuperCalc* SuperCalc_new(void);
void SuperCalc_free(SuperCalc* sc);
void SuperCalc_run(SuperCalc* sc);
Value* SuperCalc_runLine(const SuperCalc* sc, const char* str, VERBOSITY v);

#endif /* SC_SUPERCALC_H */
