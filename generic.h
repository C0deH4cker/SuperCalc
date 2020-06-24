/*
  generic.h
  SuperCalc

  Created by C0deH4cker on 11/5/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_GENERIC_H
#define SC_GENERIC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#ifdef WITH_LINENOISE
#include "linenoise/linenoise.h"
#endif

#include "annotations.h"


#ifdef _MSC_VER

# define ABS(x) ((x) < 0 ? -(x) : (x))
# define CMP(op, a, b) (((a) op (b)) ? (a) : (b))
# define HAS_ALL(flags, flag) (((flags) & (flag)) == (flag))

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

# define HAS_ALL(flags, flag) ({ \
    __typeof__(flag) _flag = (flag); \
    ((flags) & _flag) == _flag; \
})

#endif /* _MSC_VER */

#define MIN(a, b) CMP(<, a, b)
#define MAX(a, b) CMP(>, a, b)
#define CLAMP(x, lo, hi) MAX(lo, MIN(x, hi))
#define HAS_ANY(flags, flag) (((flags) & (flag)) != 0)
#define ARRSIZE(arr) (sizeof(arr) / sizeof(arr[0]))


#include "error.h"

static inline RETURNS_OWNED void* _Nonnull_unless(size == 0) fmalloc(size_t size) {
	void* ret = malloc(size);
	if(ret == NULL && size > 0) {
		allocError();
	}
	memset(ret, 0, size);
	return ret;
}

static inline RETURNS_OWNED void* _Nonnull_unless(count == 0 || size == 0) fcalloc(size_t count, size_t size) {
	void* ret = calloc(count, size);
	if(ret == NULL && count > 0 && size > 0) {
		allocError();
	}
	return ret;
}

static inline RETURNS_OWNED void* _Nonnull_unless(size == 0) frealloc(CONSUMED void* _Nullable mem, size_t size) {
	void* ret = realloc(mem, size);
	if(ret == NULL && size > 0) {
		allocError();
	}
	return ret;
}

/* Version of free() annotated to consume the pointer argument */
#define destroy(var) do { \
	free_owned(var); \
	var = CAST_NONNULL(NULL); \
} while(0)
void free_owned(CONSUMED void* _Nullable ptr);

typedef enum {
	V_NONE   = 0,
	V_ERR    = 1<<0,
	V_PRETTY = 1<<1,
	V_REPR   = 1<<2,
	V_WRAP   = 1<<3,
	V_TREE   = 1<<4,
	V_XML    = 1<<5
} VERBOSITY;

#define SC_PROMPT_NORMAL   "sc> "
#define SC_PROMPT_CONTINUE "... "
#define SC_LINE_SIZE 1000

extern char* _Nullable g_line;
extern unsigned g_lineNumber;
extern FILE* _Nullable g_inputFile;
extern const char* _Nullable g_inputFileName;


ASSUME_NONNULL_BEGIN

/* Tokenization */
void trimSpaces(istring str);
RETURNS_OWNED char* _Nullable nextSpecial(istring expr);
RETURNS_OWNED char* _Nullable nextToken(istring expr);
int getSign(istring expr);

/* Input */
RETURNS_UNOWNED char* _Nullable nextLine(const char* prompt);
bool isInteractive(FILE* fp);
VERBOSITY getVerbosity(INOUT UNOWNED istring str);

/* Verbose printing */
const char* _Nullable_unless(name != NULL) getPretty(const char* _Nullable name);
const char* indentation(unsigned level);

/* Math */
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
double approx(double real);

ASSUME_NONNULL_END

#endif /* SC_GENERIC_H */
