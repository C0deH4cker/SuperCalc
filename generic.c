/*
  generic.c
  SuperCalc

  Created by C0deH4cker on 11/5/13.
  Copyright (c) 2013 C0deH4cker. All rights reserved.
*/

#include "generic.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if READLINE
#include <readline/readline.h>
#include <readline/history.h>
char *line = NULL;
#else
#include "supercalc.h"
#endif

bool prettyPrint = false;

void nextLine(FILE* fp) {
	if(isInteractive(fp)) {
#if READLINE
		if (line) {
			/* There has to be a better way. */
			free(line);
		}
		line = readline(">>> ");
		add_history(line);
		return;
#else
		printf(">>> ");
#endif
	}
	
	fgets(line, sizeof(line), fp);
}

bool isInteractive(FILE* fp) {
	if(isatty(fileno(fp)))
		return true;
	
	return false;
}

static const char* _ugly_str[] = {
	"sqrt",
	"alpha", "beta", "gamma", "delta",
	"epsilon", "zeta", "eta", "theta",
	"iota", "kappa", "lambda", "mu",
	"nu", "xi", "omicron", "pi",
	"rho", "sigma", "tau", "upsilon",
	"phi", "chi", "psi", "omega"
};
static const char* _pretty_str[] = {
	"√",
	"α", "β", "γ", "δ",
	"ε", "ζ", "η", "θ",
	"ι", "κ", "λ", "μ",
	"ν", "ξ", "ο", "π",
	"ρ", "σ", "τ", "υ",
	"φ", "χ", "ψ", "ω"
};
const char* getPretty(const char* name) {
	if(name == NULL)
		return NULL;
	
	/* More: "∞≠π∑ß∂ƒ∆÷≥≤∫√≈±∏Ø" */
	unsigned i;
	for(i = 0; i < sizeof(_ugly_str) / sizeof(_ugly_str[0]); i++) {
		if(strcmp(name, _ugly_str[i]) == 0)
			return _pretty_str[i];
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
		if(exp & 1)
			result *= base;
		
		exp >>= 1;
		base *= base;
	}
	return result;
}

long long gcd(long long a, long long b) {
	return b == 0 ? a : gcd(b, a % b);
}

char* nextToken(const char** expr) {
	size_t len = 1;
	
	trimSpaces(expr);
	
	const char* p = *expr;
	
	/* First char must match [a-zA-Z_] */
	if(!(isalpha(p[0]) || p[0] == '_'))
		return NULL;
	
	/* Count consecutive number of chars matching [a-zA-Z0-9_] */
	while(isalnum(p[len]) || p[len] == '_')
		len++;
	
	/* TODO: See if this is the same as ret = strndup(*expr, len); */
	char* ret = fmalloc((len + 1) * sizeof(*ret));
	
	strncpy(ret, *expr, len);
	ret[len] = '\0';
	
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
	/* "Close enough" to zero */
	if(ABS(real) < EPSILON)
		return 0;
	
	return real;
}


