/*
  support.h
  SuperCalc

  Created by C0deH4cker on 9/1/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#ifndef SC_SUPPORT_H
#define SC_SUPPORT_H

#include <stdarg.h>

#ifdef _MSC_VER
# define NORETURN __declspec("noreturn")
# define UNREACHABLE() __assume(false)
#else
# define NORETURN __attribute__((__noreturn__))
# define UNREACHABLE() __builtin_unreachable()
#endif

#ifdef _MSC_VER

/* Not defined when using MSVC */
int asprintf(OUT NONNULL_WHEN(return >= 0) char* OWNED NONNULL* buffer, NONNULL const char* fmt, ...);
int vasprintf(OUT NONNULL_WHEN(return >= 0) char* OWNED NONNULL* buffer, NONNULL const char* fmt, va_list ap);

#endif /* _MSC_VER */

#endif /* SC_SUPPORT_H */
