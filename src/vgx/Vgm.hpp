// Basic vgm code, loads, saves, returns event length
// Deadfish Shitware, 2010-2012
#ifndef _VGMFILE_H_
#define _VGMFILE_H_
#include "dataBlock.hpp"
#include "vgx_.hpp"

class vgx_fileIo;
class vgm_header;
class VgmFile_ : public VgxFile_
{
public:
	static int ELen(unsigned char c);
	bool Load(vgx_fileIo& fp);
	bool Save(vgx_fileIo& fp);
	
	DataBlock dataBlock;
};

#endif
