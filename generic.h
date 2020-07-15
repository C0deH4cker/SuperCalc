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
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#ifdef WITH_LINENOISE
#include "linenoise/linenoise.h"
#endif

#include "annotations.h"


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

#define METHOD_debugString(cls) \
RETURNS_OWNED char* cls##_debugString(cls* self)

/* Whenever a new class is added, this should be incremented */
#define NUM_CLASSES 16

#ifdef WITH_OBJECT_COUNTS

typedef struct Metaclass Metaclass;
typedef struct ObjectReferences ObjectReferences;

typedef char* _Nonnull Fptr_debugString(const void* _Nonnull obj);

struct Metaclass {
	/*! Name of this class */
	const char* _Nonnull name;
	
	/*! Function to get debug information about an instance of this class */
	Fptr_debugString* _Nonnull fn_debugString;
	
	/*! Number of times that an instance of this class was statically allocated (intentionally leaked) */
	uint64_t statics;
	
	/*! Number of times that an instance of this class was non-statially allocated */
	uint64_t allocs;
	
	/*! Number of times that an instance of this class was freed */
	uint64_t frees;
	
	/*! Pointer to a linked list with detailed allocation info for each instance of this class */
	ObjectReferences* _Nullable refs;
};

struct ObjectReferences {
	/*! Pointer to the next node in the linked list */
	ObjectReferences* _Nullable next;
	
	/*! Pointer to the allocated object */
	void* _Nonnull obj;
	
	/*! Unique identifier (incrementing) assigned to each allocated object */
	uint64_t ordinal;
	
	/*! Initially set to true, then changed to false when freed */
	bool alive;
};

extern bool g_allocStatic;
extern Metaclass* _Nonnull g_all_classes[NUM_CLASSES];

#define DECL(cls) \
extern Metaclass g_##cls##_meta

#define DEF(cls) \
METHOD_debugString(cls); \
Metaclass g_##cls##_meta = {#cls, (Fptr_debugString*)&cls##_debugString, 0, 0, 0, NULL}

#define OBJECT_ALLOC(cls) ((cls*)_object_alloc(sizeof(cls), &g_##cls##_meta))
#define OBJECT_FREE(cls, ptr) _object_free((ptr), &g_##cls##_meta)

void show_all_counts(void);

#else /* WITH_OBJECT_COUNTS */

typedef void Metaclass;

#define DECL(cls)
#define DEF(cls)
#define OBJECT_ALLOC(cls) ((cls*)_object_alloc(sizeof(cls), NULL))
#define OBJECT_FREE(cls, ptr) _object_free((ptr), NULL)

#define show_all_counts()

#endif /* WITH_OBJECT_COUNTS */

bool check_leaks(void);

#include "error.h"


static inline RETURNS_OWNED void* _Nonnull fcalloc(size_t count, size_t size) {
	void* ret = calloc(count, size);
	if(ret == NULL) {
		if(count > 0 && size > 0) {
			allocError();
		}
		
		/*
		 * Requested a zero sized allocation and the allocator returned a NULL pointer, which is allowed
		 * by the spec. We don't want this function to return a NULL pointer though, so instead request a
		 * minimum sized allocation (will probably be rounded up to 16 bytes).
		 */
		ret = calloc(1, 1);
		if(ret == NULL) {
			allocError();
		}
	}
	
	return ret;
}

static inline RETURNS_OWNED void* _Nonnull fmalloc(size_t size) {
	/* Force fmalloc() to return a zero-initialized allocation by proxying to fcalloc() */
	return fcalloc(1, size);
}

static inline RETURNS_OWNED void* _Nonnull frealloc(CONSUMED void* _Nullable mem, size_t size) {
	void* ret = realloc(mem, size);
	if(ret == NULL) {
		if(size > 0) {
			allocError();
		}
		
		/*
		 * If resizing to a zero-sized allocation and the allocator returned a NULL pointer,
		 * instead return a pointer to a minimally sized allocation.
		 */
		ret = fmalloc(1);
	}
	return ret;
}

/* Version of free() annotated to consume the pointer argument */
void free_owned(CONSUMED void* _Nullable ptr);

/* Wrapper around free_destroy() that also sets the variable being freed to NULL */
#define destroy(var) do { \
	free_owned(var); \
	var = CAST_NONNULL(NULL); \
} while(0)

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

extern char* _Nullable g_line;
extern unsigned g_lineNumber;
extern FILE* _Nullable g_inputFile;
extern const char* _Nullable g_inputFileName;


ASSUME_NONNULL_BEGIN

void Breakpoint(void);

/* Object lifetime management */
RETURNS_OWNED static inline void* _object_alloc(size_t size, Metaclass* _Nullable meta) {
#ifndef WITH_OBJECT_COUNTS
	UNREFERENCED_PARAMETER(meta);
#endif /* WITH_OBJECT_COUNTS */
	
	static bool s_checkedEnv = false;
	static char* s_breakClass = NULL;
	static uint64_t s_breakOrdinal = 0;
	if(!s_checkedEnv) {
		s_checkedEnv = true;
		
		char* name = getenv("SC_BREAK_CLASS");
		if(name != NULL) {
			char* ordstr = getenv("SC_BREAK_ORDINAL");
			if(ordstr == NULL) {
				fprintf(stderr, "Must set both env vars SC_BREAK_CLASS and SC_BREAK_ORDINAL!\n");
				Breakpoint();
				abort();
			}
			
			char* end = NULL;
			uint64_t ordinal = strtoull(ordstr, &end, 10);
			if(*end != '\0') {
				fprintf(stderr, "SC_BREAK_ORDINAL must hold a positive decimal integer, not '%s'\n", ordstr);
				Breakpoint();
				abort();
			}
			
			s_breakClass = strdup(name);
			s_breakOrdinal = ordinal;
		}
	}
	
#ifdef WITH_OBJECT_COUNTS
	if(meta != NULL) {
		if(g_allocStatic) {
			++meta->statics;
		}
		else {
			++meta->allocs;
		}
	}
#endif /* WITH_OBJECT_COUNTS */
	
	void* ret = fmalloc(size);
	
#ifdef WITH_OBJECT_COUNTS
	/* Insert linked list node to track this allocated object */
	if(!g_allocStatic) {
		ObjectReferences* ref = fmalloc(sizeof(*ref));
		ref->obj = ret;
		ref->ordinal = meta->allocs;
		ref->alive = true;
		ref->next = meta->refs;
		meta->refs = ref;
		
		if(ref->ordinal == s_breakOrdinal && s_breakClass != NULL && strcmp(s_breakClass, meta->name) == 0) {
			Breakpoint();
		}
		
		ref = NULL;
	}
#endif /* WITH_OBJECT_COUNTS */
	
	return ret;
}

#ifdef WITH_OBJECT_COUNTS
static inline ObjectReferences* _Nullable * _Nonnull _object_findRef(void* _Nonnull ptr, Metaclass* _Nonnull meta) {
	/* Traverse linked list looking for the object being freed */
	ObjectReferences** pRef = &meta->refs;
	while(*pRef != NULL) {
		if((*pRef)->obj == ptr) {
			break;
		}
		pRef = &(*pRef)->next;
	}
	
	return pRef;
}
#endif /* WITH_OBJECT_COUNTS */

static inline void _object_free(void* _Nullable ptr, Metaclass* _Nullable meta) {
#ifndef WITH_OBJECT_COUNTS
	UNREFERENCED_PARAMETER(meta);
#endif /* WITH_OBJECT_COUNTS */
	
	if(ptr == NULL) {
		return;
	}
	
#ifdef WITH_OBJECT_COUNTS
	if(!g_allocStatic && meta != NULL) {
		ObjectReferences** pRef = _object_findRef(CAST_NONNULL(ptr), CAST_NONNULL(meta));
		if(*pRef == NULL) {
			/* Object isn't tracked as allocated of this type. Check for type confusion */
			unsigned i;
			for(i = 0; i < ARRSIZE(g_all_classes); i++) {
				Metaclass* cls = g_all_classes[i];
				if(cls == meta) {
					continue;
				}
				
				pRef = _object_findRef(CAST_NONNULL(ptr), CAST_NONNULL(cls));
				if(*pRef != NULL) {
					fprintf(stderr, "Type confusion during free! Tried to free an object of type %s as a %s.\n", cls->name, meta->name);
					
					char* debugstr = cls->fn_debugString(CAST_NONNULL(ptr));
					fprintf(stderr, "Object being freed: %s\n", debugstr);
					Breakpoint();
					destroy(debugstr);
					abort();
				}
			}
			
			fprintf(stderr, "Tried to free an object that isn't tracked as allocated!\n");
			Breakpoint();
			abort();
		}
		
		if(!(*pRef)->alive) {
			fprintf(stderr, "Double free detected on object of type %s!\n", meta->name);
			
			char* debugstr = meta->fn_debugString(CAST_NONNULL(ptr));
			fprintf(stderr, "Object being freed: %s\n", debugstr);
			Breakpoint();
			destroy(debugstr);
			abort();
		}
		
		/* Soft delete this linked list node */
		(*pRef)->alive = false;
	}
#endif /* WITH_OBJECT_COUNTS */
	
	/* Actually free the object */
	free_owned(ptr);
	
#ifdef WITH_OBJECT_COUNTS
	/* Update tracking counts */
	if(meta != NULL) {
		if(g_allocStatic) {
			--meta->statics;
		}
		else {
			++meta->frees;
			
			if(meta->frees > meta->allocs) {
				fprintf(stderr, "Too many frees!\n");
				Breakpoint();
				abort();
			}
		}
	}
#endif /* WITH_OBJECT_COUNTS */
}

/* Tokenization */
void trimSpaces(istring str);
RETURNS_OWNED char* _Nullable nextSpecial(istring expr);
RETURNS_OWNED char* _Nullable nextToken(istring expr);
int getSign(istring expr);

/* Input */
RETURNS_UNOWNED char* _Nullable nextLine(const char* prompt);
bool isInteractive(FILE* fp);
VERBOSITY getVerbosity(INOUT UNOWNED istring str);

/* Verbose printing */
const char* _Nullable_unless(name != NULL) getPretty(const char* _Nullable name);
const char* indentation(unsigned level);

/* Math */
long long ipow(long long base, long long exp);
long long gcd(long long a, long long b);
double approx(double real);

ASSUME_NONNULL_END

#endif /* SC_GENERIC_H */
