#pragma once
#include "stdshit.h"
#include "ccstuff.h"

class vgx_header;
class VgxFile_
{
public:
	void Free(void);

	AutoMem<char> buffer;
	vgx_header *header;
	char *sampData;
	int sampSize;
	char *mainData;
	int mainSize;
	int loopIndex;
	char *gd3Data;
	int gd3Size;
	int samples;
	int loopSamp;
	
	char *extraData; 
	int extraSize;
	
	VgxFile_() { ZINIT; }
};


inline
void VgxFile_::Free(void)
{
	buffer.free();
}
