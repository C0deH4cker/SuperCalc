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

char* g_line = NULL;

bool isInteractive(FILE* fp) {
	return ISATTY(fileno(fp));
}

VERBOSITY getVerbosity(const char** str) {
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
		RAISE(syntaxError("Specified " #lvl " verbosity more than once."), false); \
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
				RAISE(badChar(**str), false);
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
	if(name == NULL) return NULL;
	
	/* More: "∞≠∑ß∂ƒ∆÷≥≤∫≈±∏Ø" */
	unsigned i;
	for(i = 0; i < ARRSIZE(_repr_tok); i++) {
		if(strcmp(name, _repr_tok[i]) == 0) {
			return _pretty_tok[i];
		}
	}
	
	return name;
}

void trimSpaces(const char** str) {
	const char* p = *str;
	while(*p && isspace(*p)) p++;
	*str = p;
}

const char* indentation(unsigned level) {
	/* Try just using a static array full of spaces */
	static char blanks[40 * IWIDTH + 1] = "";
	if(blanks[0] == '\0') {
		memset(blanks, ICHAR, ARRSIZE(blanks) - 1);
		blanks[ARRSIZE(blanks) - 1] = '\0';
	}
	
	if(level <= (ARRSIZE(blanks) - 1) / IWIDTH) {
		return &blanks[ARRSIZE(blanks) - 1 - level * IWIDTH];
	}
	
	/* Too much indentation for static array */
	
	/* Find target index of spaces array large enough */
	size_t index = 0;
	size_t count = (ARRSIZE(blanks) - 1) / IWIDTH;
	while(level > count) {
		++index;
		count *= 2;
	}
	
	/* Expand array if necessary */
	static size_t large_count = 0;
	static char** larger = NULL;
	if(index + 1 > large_count) {
		size_t new_count = index + 1;
		larger = frealloc(larger, new_count * sizeof(*larger));
		memset(larger + large_count, 0, (new_count - large_count) * sizeof(*larger));
		large_count = new_count;
	}
	
	/* Need to create array of spaces */
	if(larger[index] == NULL) {
		larger[index] = fmalloc(count * IWIDTH + 1);
		memset(larger[index], IWIDTH, count * IWIDTH);
		larger[index][count * IWIDTH] = '\0';
	}
	
	return &larger[index][(count - level) * IWIDTH];
}

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

char* nextSpecial(const char** expr) {
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

char* nextToken(const char** expr) {
	size_t len = 1;
	
	trimSpaces(expr);
	
	char* special = nextSpecial(expr);
	if(special) {
		return special;
	}
	
	const char* p = *expr;
	
	/* First char must match [a-zA-Z_] */
	if(!(isalpha(p[0]) || p[0] == '_')) {
		return NULL;
	}
	
	/* Count consecutive number of chars matching [a-zA-Z0-9_] */
	while(isalnum(p[len]) || p[len] == '_') {
		len++;
	}
	
	char* ret = strndup(*expr, len);
	*expr += len;
	
	return ret;
}

int getSign(const char** expr) {
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

