// Basic vgc code, loads and saves
// Deadfish Shitware, 2010-2012
#ifndef _VGCFILE_H_
#define _VGCFILE_H_
#include "ccstuff.h"
#include <string.h>

class vgx_fileIo;
class vgc_header;
class VgcFile_
{
public:
	void Free(void);
	bool Load(vgx_fileIo& fp);
	bool Save(vgx_fileIo& fp);
	
	AutoMem<char> buffer;
	vgc_header *header;
	char *mainData;
	int mainSize;
	int loopIndex;
	char *sampData;
	int sampSize;
	char *gd3Data;
	int gd3Size;
};

inline
void VgcFile_::Free(void)
{
	buffer.free();
}

#endif
