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

#include "annotations.h"
#include "support.h"

typedef struct Error Error;

/* Allocate, print, free */
#define RAISE(err, death) do { \
	bool _death = (death); \
	Error* _err = (err); \
	Error_raise(_err, _death); \
	if(_death) { \
		UNREACHABLE; \
	} \
	Error_free(_err); \
} while(0)

/* Convenience constructors */
#define ignoreError()               Error_new(ERR_IGN, "")
#define mathError(...)              Error_new(ERR_MATH, __VA_ARGS__)
#define syntaxError(col, ...)       Error_new(ERR_SYNTAX, (col), __VA_ARGS__)
#define fatalError(...)             Error_new(ERR_FATAL, __VA_ARGS__)
#define nameError(...)              Error_new(ERR_NAME, __VA_ARGS__)
#define typeError(...)              Error_new(ERR_TYPE, __VA_ARGS__)
#define runtimeError(...)           Error_new(ERR_RUNTIME, __VA_ARGS__)
#define internalError(...)          Error_new(ERR_INTERNAL, __VA_ARGS__)
#define unknownError(...)           Error_new(ERR_UNK, __VA_ARGS__)

/* Common errors */
#define kNullErrStr             "NULL pointer value."
#define kDivByZeroStr           "Division by zero."
#define kModByZeroStr           "Modulus by zero."
#define kVarNotFoundStr         "No variable named '%s' found."
#define kBadOpTypeStr           "Bad %s operand type: %d."
#define kBadCharStr             "Unexpected character: '%c'."
#define kBuiltinArgsStr         "Builtin '%s' expects %u argument%s, not %u."
#define kBuiltinNotFuncStr      "Builtin '%s' is not a function."
#define kBadConversionStr       "One or more arguments to builtin '%s' couldn't be converted to numbers."
#define kEarlyEndStr            "Premature end of input."
#define kMissingPlaceholderStr  "Missing placeholder number %u."
#define kBadImportDepthStr      "Exceeded max allowed import depth when trying to import file '%s'."
#define kImportErrorStr         "Failed to import file '%s': %s."

#define kAllocErrStr            "Unable to allocate memory."
#define kBadValStr              "Unexpected value type: %d."
#define kBadVarStr              "Unexpected variable type: %d."

#define nullError()                 unknownError(kNullErrStr)
#define zeroDivError()              mathError(kDivByZeroStr)
#define zeroModError()              mathError(kModByZeroStr)
#define varNotFound(name)           nameError(kVarNotFoundStr, (name))
#define badOpType(op, type)         typeError(kBadOpTypeStr, (op), (type))
#define badChar(s)                  (*(s) ? syntaxError((s), kBadCharStr, *(s)) : syntaxError((s), kEarlyEndStr))
#define builtinArgs(name, n1, n2)   typeError(kBuiltinArgsStr, (name), (n1), (n1) == 1 ? "" : "s", (n2))
#define builtinNotFunc(name)        typeError(kBuiltinNotFuncStr, (name))
#define badConversion(name)         typeError(kBadConversionStr, (name))
#define earlyEnd(s)                 syntaxError((s), kEarlyEndStr)
#define missingPlaceholder(n)       nameError(kMissingPlaceholderStr, (n))
#define badImportDepth(filename)    runtimeError(kBadImportDepthStr, (filename))
#define importError(filename, err)  runtimeError(kImportErrorStr, (filename), (err))

/* Death macros */
#define DIE(...)                    die(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define allocError()                DIE(kAllocErrStr)
#define badValType(type)            DIE(kBadValStr, (type))
#define badVarType(type)            DIE(kBadVarStr, (type))

typedef enum {
	ERR_IGN = 0,
	ERR_MATH,
	ERR_SYNTAX,
	ERR_FATAL,
	ERR_NAME,
	ERR_TYPE,
	ERR_RUNTIME,
	ERR_INTERNAL,
	ERR_UNK
} ERRTYPE;

/* Death function */
NORETURN void die(const char* _Nonnull file, const char* _Nonnull function, int line, const char* _Nonnull fmt, ...);


#include "generic.h"


ASSUME_NONNULL_BEGIN

struct Error {
	ERRTYPE type;
	OWNED char* msg;
	OWNED char* _Nullable filename;
	unsigned line;
	unsigned column;
};

/* Constructors */
RETURNS_OWNED Error* Error_new(ERRTYPE type, const char* fmt, ...) PRINTFLIKE(2, 3);
RETURNS_OWNED Error* Error_vnew(ERRTYPE type, const char* fmt, va_list args) PRINTFLIKE(2, 0);

/* Destructor */
void Error_free(CONSUMED Error* _Nullable err);

/* Copying */
RETURNS_OWNED Error* Error_copy(const Error* err);

/* Printing and maybe a side of suicide */
void Error_raise(const Error* err, bool forceDeath);

/* Fatal or not? */
bool Error_canRecover(const Error* err);

ASSUME_NONNULL_END

#endif /* SC_ERROR_H */
