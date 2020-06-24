/*
  generic.c
  SuperCalc

  Created by C0deH4cker on 11/5/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#ifdef _MSC_VER
# include <io.h>
# define ISATTY(fp) _isatty(fp)
#else
# include <unistd.h>
# define ISATTY(fp) isatty(fp)
#endif

#include "supercalc.h"

#define ICHAR   ' '
#define IWIDTH  2
#define EPSILON 1e-12

typedef enum {
	VC_PRETTY = 'p',
	VC_REPR   = 'r',
	VC_WRAP   = 'w',
	VC_TREE   = 't',
	VC_XML    = 'x'
} VERBOSITY_CHAR;


void free_owned(void* ptr) {
	free(ptr);
}

char* g_line = NULL;
unsigned g_lineNumber = 0;
FILE* g_inputFile = NULL;
const char* g_inputFileName = "<interactive>";

char* nextLine(const char* prompt) {
#ifdef WITH_LINENOISE
	/* Only use linenoise for the interactive prompt, not for imported files */
	if(g_inputFile == NULL) {
		destroy(g_line);
		g_line = linenoise(prompt);
		if(g_line != NULL) {
			linenoiseHistoryAdd(g_line);
		}
		
		/* Strip trailing newline and comments */
		g_line = strsep(&g_line, "#\r\n");
		++g_lineNumber;
		return g_line;
	}
#endif /* WITH_LINENOISE */
	
	/* Allocate a line buffer if it hasn't been yet */
	if(g_line == NULL) {
		g_line = fmalloc(SC_LINE_SIZE);
	}
	
	/* If there is an explicit input file, don't print the prompt for every line */
	FILE* fp = g_inputFile;
	if(fp == NULL) {
		printf("%s", prompt);
		fp = stdin;
	}
	
	/* Read one line from the input stream (file or stdin) */
	if(fgets(g_line, SC_LINE_SIZE, fp) == NULL) {
		return NULL;
	}
	
	/* Strip trailing newline and comments */
	g_line = strsep(&g_line, "#\r\n");
	++g_lineNumber;
	return g_line;
}

bool isInteractive(FILE* fp) {
	return ISATTY(fileno(fp));
}

VERBOSITY getVerbosity(istring str) {
	VERBOSITY ret = V_NONE;
	
	if(**str != '?') {
		return ret;
	}
	
	/* Move past the '?' */
	(*str)++;
	
	bool again = true;
	while(again) {
		switch(**str) {
#define ADD_V(lvl) do { \
	if(ret & V_##lvl) { \
		RAISE(syntaxError(*str, "Specified " #lvl " verbosity more than once."), false); \
		return V_ERR; \
	} \
	ret |= V_##lvl; \
} while(0)
			
			case VC_REPR:
				ADD_V(REPR);
				break;
			
			case VC_PRETTY:
				ADD_V(PRETTY);
				/* Pretty implies repr */
				ret |= V_REPR;
				break;
			
			case VC_WRAP:
				ADD_V(WRAP);
				break;
			
			case VC_TREE:
				ADD_V(TREE);
				break;
			
			case VC_XML:
				ADD_V(XML);
				break;
			
			case ' ':
			case '\t':
				/* Verbosity command ended by whitespace only */
				again = false;
				break;
			
			default:
				RAISE(badChar(*str), false);
				return V_ERR;
		}
		
#undef ADD_V
		
		(*str)++;
	}
	
	/* For slight backwards compatibility and for ease of use */
	if(ret == 0) {
		ret |= V_REPR;
	}
	
	return ret;
}

static const char* _repr_tok[] = {
	"sqrt",
	"alpha", "beta", "gamma", "delta",
	"epsilon", "zeta", "eta", "theta",
	"iota", "kappa", "lambda", "mu", "mu",
	"nu", "xi", "omicron", "pi",
	"rho", "sigma", "tau", "upsilon",
	"phi", "chi", "psi", "omega"
};
static const char* _pretty_tok[] = {
	"√",
	"α", "β", "γ", "δ",
	"ε", "ζ", "η", "θ",
	"ι", "κ", "λ", "μ", "µ",
	"ν", "ξ", "ο", "π",
	"ρ", "σ", "τ", "υ",
	"φ", "χ", "ψ", "ω"
};
const char* getPretty(const char* name) {
	if(name == NULL) {
		return NULL;
	}
	
	/* More: "∞≠∑ß∂ƒ∆÷≥≤∫≈±∏Ø" */
	unsigned i;
	for(i = 0; i < ARRSIZE(_repr_tok); i++) {
		if(strcmp(name, _repr_tok[i]) == 0) {
			return _pretty_tok[i];
		}
	}
	
	return name;
}

void trimSpaces(istring str) {
	*str += strspn(*str, " \t");
}


#define FIVE_TIMES(x...) x,x,x,x,x
#define EIGHT_TIMES(x...) FIVE_TIMES(x),x,x,x
#define FORTY_TIMES(x...) FIVE_TIMES(EIGHT_TIMES(x))

#if IWIDTH == 2
#define INDENT ICHAR,ICHAR
#else /* IWIDTH */
#error This code must be changed!
#endif /* IWIDTH */

const char* indentation(unsigned level) {
	/* Use a static array full of spaces */
	static const char blanks[40 * IWIDTH + 1] = {FORTY_TIMES(INDENT)};
	
	/* If the requested indentation is too big, just cap it */
	level = MIN(level, (ARRSIZE(blanks) - 1) / IWIDTH);
	return &blanks[ARRSIZE(blanks) - 1 - level * IWIDTH];
}

#undef INDENT
#undef FORTY_TIMES
#undef EIGHT_TIMES
#undef FIVE_TIMES

long long ipow(long long base, long long exp) {
	long long result = 1;
	while(exp) {
		if(exp & 1) {
			result *= base;
		}
		
		exp >>= 1;
		base *= base;
	}
	return result;
}

long long gcd(long long a, long long b) {
	return b == 0 ? a : gcd(b, a % b);
}

char* nextSpecial(istring expr) {
	unsigned i;
	for(i = 0; i < ARRSIZE(_pretty_tok); i++) {
		size_t len = strlen(_pretty_tok[i]);
		
		if(strncmp(_pretty_tok[i], *expr, len) == 0) {
			*expr += len;
			return strdup(_repr_tok[i]);
		}
	}
	
	return NULL;
}

char* nextToken(istring expr) {
	size_t len = 1;
	
	trimSpaces(expr);
	
	if(**expr == '\0') {
		/* Request more input in cases like "3 +" */
		char* line = nextLine(SC_PROMPT_CONTINUE);
		if(line != NULL) {
			*expr = line;
		}
		else {
			return NULL;
		}
	}
	
	char* special = nextSpecial(expr);
	if(special) {
		return special;
	}
	
	const char* p = *expr;
	
	/* First char must match [a-zA-Z_] */
	if(!(isalpha(p[0]) || p[0] == '_')) {
		return NULL;
	}
	
	/* Count consecutive number of chars matching [a-zA-Z0-9_'] */
	while(isalnum(p[len]) || p[len] == '_' || p[len] == '\'') {
		len++;
	}
	
	char* ret = strndup(*expr, len);
	*expr += len;
	
	return ret;
}

int getSign(istring expr) {
	int sign = 1;
	
	trimSpaces(expr);
	
	if(**expr == '-') {
		sign = -1;
		(*expr)++;
	}
	
	return sign;
}

double approx(double real) {
	/* "Close enough" to zero. This makes %g look better. */
	if(ABS(real) < EPSILON) {
		return 0.0;
	}
	
	return real;
}

