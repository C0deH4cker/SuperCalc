/*
  support.c
  SuperCalc

  Created by C0deH4cker on 9/1/15.
  Copyright (c) 2015 C0deH4cker. All rights reserved.
*/

#include "support.h"

#ifdef _MSC_VER

int asprintf(char** buffer, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	
	int ret = vasprintf(buffer, fmt, ap);
	
	va_end(ap);
	return ret;
}

int vasprintf(char** buffer, const char* fmt, va_list ap) {
	int needed = _vscprintf(fmt, ap);
	if(needed < 0) {
		*buffer = NULL;
		return needed;
	}
	
	/* Because _vscprintf() doesn't count the NULL */
	++needed;
	
	int ret = -1;
	char* buf = malloc(needed + 1);
	if(buf) {
		ret = _vsnprintf(buf, needed, fmt, ap);
		buf[needed] = '\0';
	}
	
	*buffer = buf;
	return ret;
}

#endif
