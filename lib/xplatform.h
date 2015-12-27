#ifndef _XPLATFORM_H_
#define _XPLATFORM_H_
#include "config.h"

// native string char
#include <tchar.h>
#ifndef _NCHAR_
#define _NCHAR_
 #ifdef _UNICODE
  #define nchar wchar_t
  #define nstr(x) L##x
 #else
  #define nchar char
  #define nstr(x) x
 #endif
#endif

// native string fopen
#ifdef _UNICODE
 #define nfopen _wfopen
#else
 #define nfopen fopen
#endif

// native string entry point
#ifdef _UNICODE
 #ifdef __GNUC__
  #include <windows.h>
  #define nmain _nwmain(int argc, nchar* argv[]); \
  int main(void){ int argc; LPWSTR *argv; \
	argv = CommandLineToArgvW(GetCommandLineW(), &argc); \
	return _nwmain(argc, argv);} int _nwmain
 #else
   #define nmain wmain
 #endif
#else
 #define nmain main
#endif

// return code 
static int return_map(int in)
{
	if(in == 0)
		return 0;
	return in + 10;
}

// function attributes
#ifdef __GNUC__
 #define NORETURN __attribute__((__noreturn__))
#elif defined(_MSC_VER)
 #define NORETURN __declspec(noreturn)
#else
 #define NORETURN
#endif

// fopen error checker
int fopen_ErrChk(void);

#endif
