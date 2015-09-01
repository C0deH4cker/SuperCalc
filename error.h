/*
  error.h
  SuperCalc

  Created by C0deH4cker on 11/10/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#ifndef _SC_ERROR_H_
#define _SC_ERROR_H_

#include <stdbool.h>
#include <stdarg.h>

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
	Error* _err = (err); \
	Error_raise(_err, (death)); \
	Error_free(_err); \
} while(0)

/* Convenience constructors */
#define ignoreError()               Error_new(ERR_IGN, "")
#define mathError(fmt, args...)     Error_new(ERR_MATH, fmt, ##args)
#define syntaxError(fmt, args...)   Error_new(ERR_SYNTAX, fmt, ##args)
#define fatalError(fmt, args...)    Error_new(ERR_FATAL, fmt, ##args)
#define nameError(fmt, args...)     Error_new(ERR_NAME, fmt, ##args)
#define typeError(fmt, args...)     Error_new(ERR_TYPE, fmt, ##args)
#define internalError(fmt, args...) Error_new(ERR_INTERNAL, fmt, ##args)
#define unknownError(fmt, args...)  Error_new(ERR_UNK, fmt, ##args)

const char* kNullErrStr;
const char* kDivByZeroStr;
const char* kModByZeroStr;
const char* kVarNotFoundStr;
const char* kBadOpTypeStr;
const char* kBadCharStr;
const char* kBuiltinArgsStr;
const char* kBuiltinNotFuncStr;
const char* kBadConversionStr;
const char* kEarlyEndStr;
const char* kMissingPlaceholderStr;

const char* kAllocErrStr;
const char* kBadValStr;
const char* kBadVarStr;

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
#define DIE(args...)                die(__FILE__, __FUNCTION__, __LINE__, ##args)
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
void die(const char* file, const char* function, int line, const char* fmt, ...) __attribute__((__noreturn__));

#endif
