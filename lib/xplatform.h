#ifndef _XPLATFORM_H_
#define _XPLATFORM_H_

// native string char
#ifdef _WIN32
 #define nchar wchar_t
 #define nstr(x) L##x
#else
 #define nchar char
 #define nstr(x) x
#endif

// native string fopen
#ifdef _WIN32
 #include <wchar.h>
 #define nfopen _wfopen
 #define _stscanf swscanf
 #define _tstrlen wcslen
 #define _fgetts fgetws
 #define _tcscpy wcscpy
 #define _tprintf wprintf
 #define _stprintf swprintf
 #define _tstoi _wtoi
 #define nmain wmain
#else
 #define nfopen fopen
 #define _stscanf sscanf
 #define _tstrlen strlen
 #define _fgetts fgets
 #define _tcscpy strcpy
 #define _tprintf printf
 #define _stprintf sprintf
 #define _tstoi atoi
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

// filename stuff
static bool isSep(int ch) {
#ifdef _WIN32
  if(ch == '\\') 
    return true;
#endif
  return ch == '/';
}

int getPathLen(nchar* name);



#endif
