/*
  supercalc.h
  SuperCalc

  Created by C0deH4cker on 11/22/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SUPERCALC_H_
#define _SUPERCALC_H_

#include <stdbool.h>
#include <stdio.h>

typedef struct SuperCalc SuperCalc;
#include "value.h"
#include "context.h"

struct SuperCalc {
	Context* ctx;
	FILE* fin;
	FILE* fout;
};

SuperCalc* SC_new(FILE* fout);
void SC_free(SuperCalc* sc);
Value* SC_runFile(SuperCalc* sc, FILE* fp);
Value* SC_runString(SuperCalc* sc, const char* str);

#endif
