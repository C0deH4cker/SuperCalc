/*
  error.h
  SuperCalc

  Created by C0deH4cker on 11/10/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef SC_ERROR_H
#define SC_ERROR_H

#include <stdbool.h>
#include <stdarg.h>

#include "support.h"

typedef struct Error Error;

typedef enum {
	ERR_IGN = 0,
	ERR_MATH,
	ERR_SYNTAX,
	ERR_FATAL,
	ERR_NAME,
	ERR_TYPE,
	ERR_INTERNAL,
	ERR_UNK
} ERRTYPE;

struct Error {
	ERRTYPE type;
	char* msg;
};


/* Allocate, print, free */
#define RAISE(err, death) do { \
	bool _death = (death); \
	Error* _err = (err); \
	Error_raise(_err, _death); \
	if(_death) { \
		UNREACHABLE(); \
	} \
	Error_free(_err); \
} while(0)

/* Convenience constructors */
#define ignoreError()               Error_new(ERR_IGN, "")
#define mathError(...)              Error_new(ERR_MATH, __VA_ARGS__)
#define syntaxError(...)            Error_new(ERR_SYNTAX, __VA_ARGS__)
#define fatalError(...)             Error_new(ERR_FATAL, __VA_ARGS__)
#define nameError(...)              Error_new(ERR_NAME, __VA_ARGS__)
#define typeError(...)              Error_new(ERR_TYPE, __VA_ARGS__)
#define internalError(...)          Error_new(ERR_INTERNAL, __VA_ARGS__)
#define unknownError(...)           Error_new(ERR_UNK, __VA_ARGS__)

extern const char* kNullErrStr;
extern const char* kDivByZeroStr;
extern const char* kModByZeroStr;
extern const char* kVarNotFoundStr;
extern const char* kBadOpTypeStr;
extern const char* kBadCharStr;
extern const char* kBuiltinArgsStr;
extern const char* kBuiltinNotFuncStr;
extern const char* kBadConversionStr;
extern const char* kEarlyEndStr;
extern const char* kMissingPlaceholderStr;

extern const char* kAllocErrStr;
extern const char* kBadValStr;
extern const char* kBadVarStr;

#define nullError()                 unknownError(kNullErrStr)
#define zeroDivError()              mathError(kDivByZeroStr)
#define zeroModError()              mathError(kModByZeroStr)
#define varNotFound(name)           nameError(kVarNotFoundStr, (name))
#define badOpType(op, type)         typeError(kBadOpTypeStr, (op), (type))
#define badChar(ch)                 (ch ? syntaxError(kBadCharStr, (ch)) : syntaxError(kEarlyEndStr))
#define builtinArgs(name, n1, n2)   typeError(kBuiltinArgsStr, (name), (n1), (n1) == 1 ? "" : "s", (n2))
#define builtinNotFunc(name)        typeError(kBuiltinNotFuncStr, (name))
#define badConversion(name)         typeError(kBadConversionStr, (name))
#define earlyEnd()                  syntaxError(kEarlyEndStr)
#define missingPlaceholder(n)       nameError(kMissingPlaceholderStr, (n))

/* Death macros */
#define DIE(...)                    die(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define allocError()                DIE(kAllocErrStr)
#define badValType(type)            DIE(kBadValStr, (type))
#define badVarType(type)            DIE(kBadVarStr, (type))

/* Constructors */
Error* Error_new(ERRTYPE type, const char* fmt, ...);
Error* Error_vnew(ERRTYPE type, const char* fmt, va_list args);

/* Destructor */
void Error_free(Error* err);

/* Copying */
Error* Error_copy(const Error* err);

/* Printing and maybe a side of suicide */
void Error_raise(const Error* err, bool forceDeath);

/* Fatal or not? */
bool Error_canRecover(const Error* err);

/* Death function */
NORETURN void die(const char* file, const char* function, int line, const char* fmt, ...);

#endif /* SC_ERROR_H */
