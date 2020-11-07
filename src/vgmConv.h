#ifndef _VGMCONV_H_
#define _VGMCONV_H_
#ifdef __cplusplus
extern "C" {
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

// native string char
#ifdef _WIN32
	typedef wchar_t vgmConv_char;
#else
	typedef char vgmConv_char;
#endif

// vgmConv interface
typedef struct
{
	unsigned dupRemove;
	double sscale;
	vgmConv_char* romName;
	int romLimit;
	
	vgmConv_char* source;
	union{
		vgmConv_char* dest;
		char* fileData;};
	union{
		int options;
		int fileSize;};
}_vgmConv;

int vgmConv(_vgmConv* in_out);
const char* vgmConv_errStr(int err);
int vgmRom(_vgmConv* in_out, vgmConv_char* romFile);

#ifdef __cplusplus 
}
#endif
#endif
