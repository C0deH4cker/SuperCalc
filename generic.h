/*
  generic.h
  SuperCalc

  Created by C0deH4cker on 11/5/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_GENERIC_H_
#define _SC_GENERIC_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"


#define ABS(x) ({ \
	__typeof__(x) _x = (x); \
	_x < 0 ? -_x : _x; \
})

#define CMP(op, a, b) ({ \
	__typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	(_a op _b) ? _a : _b; \
})
#define MIN(a, b) CMP(<, a, b)
#define MAX(a, b) CMP(>, a, b)

#define CLAMP(x, lo, hi) MAX(lo, MIN(x, hi))

#define ARRSIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define fmalloc(size) ({ \
	void* _mem = malloc((size)); \
	if(_mem == NULL) allocError(); \
	_mem; \
})

#define frealloc(mem, size) ({ \
	__typeof__(mem) _mem = realloc((mem), (size)); \
	if(_mem == NULL) allocError(); \
	_mem; \
})

#define IWIDTH 2
#define EPSILON 1e-12

typedef enum {
	V_REPR   = 1,
	V_TREE   = 1<<1,
	V_PRETTY = 1<<2
} VERBOSITY;
#define V_ALL (V_REPR|V_TREE|V_PRETTY)

/* Hacky, I know */
char line[1024];

/* Tokenization */
void trimSpaces(const char** str);
char* nextSpecial(const char** expr);
char* nextToken(const char** expr);
char* copyUntilClose(const char** expr);
int getSign(const char** expr);

/* Input */
void nextLine(FILE* fp);
void readLine(FILE* fp);
bool isInteractive(FILE* fp);
VERBOSITY getVerbosity(const char** str);

/* Verbose printing */
const char* getPretty(const char* name);
char* spaces(int n);
char* strNULL(void);
char* strERR(void);

/* Math */
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
double approx(double real);

#endif
