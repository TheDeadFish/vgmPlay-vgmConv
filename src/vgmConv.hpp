#ifndef _VGMCONV_HPP_
#define _VGMCONV_HPP_
#include "vgmConv.h"
#include "vgx/Vgx.hpp"

#define vgmFormat 1
#define vgcFormat 2
#define removeDup 4
#define writeInitB 8
#define compressO 16
#define avSamp 32

struct VgmConv : _vgmConv
{
	int vgmConvert(vgmFile& vgmInfo);
	int vgcConvert(vgcFile& vgcInfo);
};

#endif
