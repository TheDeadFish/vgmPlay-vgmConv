#ifndef _STDSHIT_H_
#define _STDSHIT_H_

#define ei else if
#define DEF_RETPAIR(name, T, a, U, b) \
struct name { T a; U b; name() {} \
	name(T ai) { a = ai; } \
	name(T ai, U bi) { a = ai; b = bi; } \
	operator T&() { return a; } };
typedef unsigned char 	byte;
typedef unsigned int 	uint;

#endif
