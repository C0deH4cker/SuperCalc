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

#ifdef WITH_LINENOISE
#include "linenoise/linenoise.h"
#endif


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

#define UNREFERENCED_PARAMETER(param) do { param = param; } while(0)

/* Attributes purely for documentation purposes */
#define IN
#define OUT
#define INOUT

#define NONNULL
#define NULLABLE
#define OWNED
#define UNOWNED

#define INVARIANT(...)
#define NONNULL_WHEN(...)
#define NULLABLE_WHEN(...)
#define OWNED_WHEN(...)
#define UNOWNED_WHEN(...)


#include "error.h"

static inline OWNED NONNULL_WHEN(size > 0) void* fmalloc(size_t size) {
	void* ret = malloc(size);
	if(ret == NULL && size > 0) {
		allocError();
	}
	memset(ret, 0, size);
	return ret;
}

static inline OWNED NONNULL_WHEN(count > 0 && size > 0) void* fcalloc(size_t count, size_t size) {
	void* ret = calloc(count, size);
	if(ret == NULL && count > 0 && size > 0) {
		allocError();
	}
	return ret;
}

static inline OWNED NONNULL_WHEN(size > 0) void* frealloc(OWNED NULLABLE void* mem, size_t size) {
	void* ret = realloc(mem, size);
	if(ret == NULL && size > 0) {
		allocError();
	}
	return ret;
}

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

extern char* g_line;
extern unsigned g_lineNumber;
extern FILE* g_inputFile;
extern const char* g_inputFileName;

/* Tokenization */
void trimSpaces(NONNULL const char** str);
OWNED NULLABLE char* nextSpecial(NONNULL const char** expr);
OWNED NULLABLE char* nextToken(NONNULL const char** expr);
int getSign(NONNULL const char** expr);

/* Input */
UNOWNED char* nextLine(NONNULL const char* prompt);
bool isInteractive(NONNULL FILE* fp);
VERBOSITY getVerbosity(INOUT UNOWNED NONNULL char** str);

/* Verbose printing */
const char* getPretty(NULLABLE const char* name);
const char* indentation(unsigned level);

/* Math */
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
double approx(double real);

#endif /* SC_GENERIC_H */
