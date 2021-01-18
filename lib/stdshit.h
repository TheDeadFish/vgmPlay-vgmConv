#ifndef _STDSHIT_H_
#define _STDSHIT_H_
#include <stdlib.h>
#include <string.h>
#include "xplatform.h"
#include <type_traits>

#define ei else if
#define DEF_RETPAIR(name, T, a, U, b) \
struct name { T a; U b; name() {} \
	name(T ai) { a = ai; } \
	name(T ai, U bi) { a = ai; b = bi; } \
	operator T&() { return a; } };
typedef unsigned char 	byte;
typedef unsigned short	u16;
typedef unsigned int 	uint;

char* load_file(nchar* name, size_t& size);

#define TMPL(T) template <class T>
TMPL(T) std::make_unsigned_t<T> uns(T v) { return v; }

int removeCrap(nchar* str);

#define IFRET(xx) if(auto x = xx) return x;

static int bswap32(int x) {
	return __builtin_bswap32(x); }
	
int strsize(const nchar* str);

#define ZINIT memset(this, 0, sizeof(*this))

nchar* replName(const nchar* path, const nchar* name);

__thiscall
void* xNextAlloc_(void* ptr, uint& count, uint size);
template<class T>
T& xNextAlloc(T*& ptr, uint& count) {
	return *(T*)xNextAlloc_(&ptr, count, sizeof(T)); }


// error handling
[[noreturn]] void fatal_error(const char*fmt,...);
[[noreturn]] void load_error(const char* type, const char* fileName);
[[noreturn]] void file_corrupt(const char* type, const char* fileName);
[[noreturn]] void file_bad(const char* type, const char* fileName);
[[noreturn]] void recurse_error(const char* type);
	
#define error_msg(fmt, ...) \
	fprintf(stderr, fmt, __VA_ARGS__);
	
#define ZINIT memset(this, 0, sizeof(*this))

template <class T, class U, class V> constexpr bool inRng(
	T t, U u, V v) { return (t >= u)&&(t <= v); }

// std::swap is broken
template <class T>
void swap(T& x, T& y) {
	auto tmp = x; x = y; y = tmp; }

#endif
