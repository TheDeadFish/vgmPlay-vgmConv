#ifndef _STDSHIT_H_
#define _STDSHIT_H_
#include <stdlib.h>
#include "xplatform.h"
#include <type_traits>

#define ei else if
#define DEF_RETPAIR(name, T, a, U, b) \
struct name { T a; U b; name() {} \
	name(T ai) { a = ai; } \
	name(T ai, U bi) { a = ai; b = bi; } \
	operator T&() { return a; } };
typedef unsigned char 	byte;
typedef unsigned int 	uint;

char* load_file(nchar* name, size_t& size);

#define TMPL(T) template <class T>
TMPL(T) std::make_unsigned_t<T> uns(T v) { return v; }

int removeCrap(nchar* str);

#define IFRET(xx) if(auto x = xx) return x;

static int bswap32(int x) {
	return __builtin_bswap32(x); }

#endif
