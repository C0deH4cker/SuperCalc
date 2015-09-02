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


#ifdef _MSC_VER

# define ABS(x) ((x) < 0 ? -(x) : (x))
# define CMP(op, a, b) (((a) op (b)) ? (a) : (b))

#else /* _MSC_VER */

# define ABS(x) ({ \
	__typeof__(x) _x = (x); \
	_x < 0 ? -_x : _x; \
})

# define CMP(op, a, b) ({ \
	__typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	(_a op _b) ? _a : _b; \
})

#endif /* _MSC_VER */

#define MIN(a, b) CMP(<, a, b)
#define MAX(a, b) CMP(>, a, b)
#define CLAMP(x, lo, hi) MAX(lo, MIN(x, hi))

#define ARRSIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static inline void* fmalloc(size_t size) {
	void* ret = malloc(size);
	if(ret == NULL) {
		allocError();
	}
	return ret;
}

static inline void* fcalloc(size_t count, size_t size) {
	void* ret = calloc(count, size);
	if(ret == NULL) {
		allocError();
	}
	return ret;
}

static inline void* frealloc(void* mem, size_t size) {
	void* ret = realloc(mem, size);
	if(ret == NULL) {
		allocError();
	}
	return ret;
}

typedef enum {
	V_REPR   = 1<<0,
	V_PRETTY = 1<<1,
	V_TREE   = 1<<2,
	V_XML    = 1<<3
} VERBOSITY;

/* Hacky, I know */
char line[4096];

/* Tokenization */
void trimSpaces(const char** str);
char* nextSpecial(const char** expr);
char* nextToken(const char** expr);
int getSign(const char** expr);

/* Input */
void readLine(FILE* fp, const char* prompt);
bool isInteractive(FILE* fp);
VERBOSITY getVerbosity(const char** str);

/* Verbose printing */
const char* getPretty(const char* name);
const char* indentation(unsigned level);

/* Math */
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
double approx(double real);

#endif /* _SC_GENERIC_H_ */
