// Basic vgm code, loads, saves, returns event length
// Deadfish Shitware, 2010-2012
#ifndef _VGMFILE_H_
#define _VGMFILE_H_
#include "dataBlock.hpp"
#include "ccstuff.h"

class vgx_fileIo;
class vgm_header;
class VgmFile_
{
public:
	void Free(void);
	static int ELen(unsigned char c);
	bool Load(vgx_fileIo& fp);
	bool Save(vgx_fileIo& fp);
	
	AutoMem<char> buffer;
	vgm_header *header;
	char *sampData;
	int sampSize;
	char *mainData;
	int mainSize;
	int loopIndex;
	char *gd3Data;
	int gd3Size;
	int samples;
	int loopSamp;
	DataBlock dataBlock;
		
	char* getLoop() {
		if(loopIndex != 0xffffffff) {
			return mainData + loopIndex;
		} else return NULL;
	}
};

inline
void VgmFile_::Free(void)
{
	buffer.free();
}

#endif
