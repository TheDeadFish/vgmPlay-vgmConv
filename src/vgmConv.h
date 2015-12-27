#ifndef _VGMCONV_H_
#define _VGMCONV_H_
#include "config.h"
#ifdef __cplusplus
extern "C" {
#endif

// windows unicode support
// native string char
#ifndef _NCHAR_
#define _NCHAR_
 #ifdef _UNICODE
  #include <wchar.h>
  #define nchar wchar_t
  #define nstr(x) L##x
 #else
  #define nchar char
  #define nstr(x) x
 #endif
#endif

// vgmConv options
#define vgmConv_vgmFmt	1
#define vgmConv_vgcFmt 	2
#define vgmConv_remDup 	4
#define vgmConv_wrInit 	8
#define vgmConv_zPack	16
#define vgmConv_avSamp	32

// vgmConv error codes
#define vgmConv_fileOk	0
#define vgmConv_fileBad 1
#define vgmConv_srcErr	2
#define vgmConv_dstErr	3
#define vgmConv_ReadErr 4
#define vgmConv_WritErr 5
#define vgmConv_UrecTyp 6
#define vgmConv_MemErr	7
#define vgmConv_Undef	-1

// vgmConv interface
typedef struct
{
	unsigned dupRemove;
	double sscale;
	nchar* source;
	union{
		nchar* dest;
		char* fileData;};
	union{
		int options;
		int fileSize;};
}_vgmConv;
int vgmConv(_vgmConv* in_out);
const char* vgmConv_errStr(int err);

#ifdef __cplusplus 
}
#endif
#endif
