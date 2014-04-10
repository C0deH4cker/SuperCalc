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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#include "supercalc.h"


void nextLine(FILE* fp) {
	fgets(line, sizeof(line), fp);
}

void readLine(FILE* fp) {
	printf(">>> ");
	fgets(line, sizeof(line), fp);
}

bool isInteractive(FILE* fp) {
	return isatty(fileno(fp));
}

VERBOSITY getVerbosity(const char** str) {
	VERBOSITY ret = 0;
	
	while(**str == '?') {
		ret = (ret << 1) | 1;
		(*str)++;
	}
	
	return ret & V_ALL;
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

char* spaces(int n) {
	char* ret = fmalloc((n + 1) * sizeof(*ret));
	
	memset(ret, ' ', n);
	ret[n] = '\0';
	return ret;
}

char* strNULL(void) {
	return strdup("NULL");
}

char* strERR(void) {
	return strdup("ERR");
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

/* Don't think this is needed any more, but it doesn't hurt to keep it around. */
char* copyUntilClose(const char** expr) {
	char* ret;
	
	/* Called when expr is after the opening parenthesis */
	
	int sublen;
	int level = 1;
	/* We need a reference to the beginning of the parenthesized subexpression */
	const char* ex = *expr;
	
	/* Skip opening parenthesis */
	(*expr)++;
	
	/* Calculate length of substring between parentheses */
	for(sublen = 1; **expr; (*expr)++, sublen++) {
		if(**expr == '(') {
			level++;
		}
		else if(**expr == ')') {
			level--;
			
			/* If the current parenthesis level was just closed, break from the loop */
			if(level == 0) {
				/* Skip final closing parenthesis */
				(*expr)++;
				break;
			}
		}
		else if(**expr == '\r' || **expr == '\n') {
			/* Treat EOL as end of subexpression */
			(*expr)++;
			break;
		}
	}
	/* Subtract one from length due to final parenthesis or EOL */
	sublen--;
	
	/* Create the substring */
	ret = fmalloc((sublen + 1) * sizeof(*ret));
	
	/* Copy the contents of the parenthesized group to a new expression string */
	strncpy(ret, &ex[1], sublen);
	ret[sublen] = '\0';
	
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

