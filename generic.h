/*
  generic.h
  SuperCalc

  Created by C0deH4cker on 11/5/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_GENERIC_H_
#define _SC_GENERIC_H_

#include <stddef.h>
#include <stdlib.h>
#include "error.h"

#define ABS(x) ({ \
	__typeof__(x) _x = (x); \
	_x < 0 ? -_x : _x; \
})

#define fmalloc(size) ({ \
	void* _mem = malloc((size)); \
	if(_mem == NULL) allocError(); \
	_mem; \
})

#define frealloc(mem, size) ({ \
	void* _mem = realloc((mem), (size)); \
	if(_mem == NULL) allocError(); \
	_mem; \
})

#define IWIDTH 2

char line[1024];

char* nextLine(void);
void trimSpaces(const char** str);
char* spaces(int n);
char* strNULL(void);
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
char* nextToken(const char** expr);
char* copyUntilClose(const char** expr);
int getSign(const char** expr);

#endif
