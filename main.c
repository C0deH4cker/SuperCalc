/*
  main.c
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "supercalc.h"


int main(int argc, char* argv[]) {
	SuperCalc* sc = SC_new(stdout);
	SC_runFile(sc, stdin);
	SC_free(sc);
	
	return 0;
}