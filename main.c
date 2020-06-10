/*
  main.c
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "supercalc.h"
#include <stdio.h>

int main(void) {
	SuperCalc* sc = SuperCalc_new();
	SuperCalc_run(sc);
	SuperCalc_free(sc);
	
	return 0;
}
