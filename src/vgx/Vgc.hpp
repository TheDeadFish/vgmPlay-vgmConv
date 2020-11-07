// Basic vgc code, loads and saves
// Deadfish Shitware, 2010-2012
#ifndef _VGCFILE_H_
#define _VGCFILE_H_
#include "vgx_.hpp"

class vgx_fileIo;
class vgc_header;
class VgcFile_ : public VgxFile_
{
public:
	bool Load(vgx_fileIo& fp);
	bool Save(vgx_fileIo& fp);
};

#endif
