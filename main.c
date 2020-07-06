/*
  main.c
  SuperCalc

  Created by C0deH4cker on 10/20/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "supercalc.h"
#include <stdio.h>
#include <unistd.h>

#define PROFILING 0

int main(int argc, char** argv) {
#if PROFILING
	sleep(1);
#endif /* PROFILING */
	
	SuperCalc* sc = SuperCalc_new();
	
	if(argc > 1) {
		int i;
		for(i = 1; i < argc; i++) {
			Error* err = SuperCalc_importFile(sc, argv[i]);
			if(err != NULL) {
				Error_raise(err, true);
				UNREACHABLE;
			}
		}
	}
	
#if !PROFILING
	SuperCalc_run(sc);
#endif /* PROFILING */
	
	SuperCalc_free(sc);
	
	if(check_leaks()) {
		abort();
	}
	
#if PROFILING
	sleep(10);
#endif /* PROFILING */
	return 0;
}
