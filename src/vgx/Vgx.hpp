#ifndef _VGX_H_
#define _VGX_H_
#include "Vgm.hpp"
#include "Vgc.hpp"
#include "xplatform.h"
#include <stdio.h>

// zlib support
#ifdef HAS_ZLIB
#include <zlib.h>
#define _fpz gzFile
#else
#define _fpz FILE*
#endif

// status codes
enum vgx_Status
{
	VGX_FILE_OK,
	VGX_FILE_BAD,
	VGX_ROPEN_ERR,
	VGX_WOPEN_ERR,
	VGX_READ_ERR,
	VGX_WRITE_ERR,
	VGX_MEM_ERR,
	// types
	VGX_UREC_TYPE,
	VGX_VGM_TYPE,
	VGX_VGC_TYPE
};

class vgx_fileIo
{
public:
	vgx_fileIo();
	~vgx_fileIo();
	void Close(void);
	
	// file / mem, functions
	enum{ rb, wb, wbz, mw};
	bool Open(const nchar* name, char _mode);
	bool Read(void* buff, int len);
	bool Write(void* buff, int len);
	
	// memory functions
	bool Reserve(int reqSize);
	char* Release(int& memSize);
	
	// Error handling
	vgx_Status status;
	void FileBad(void);
	void AllocErr(void);

private:
	union{
		char* poit;
		FILE* fp;
		_fpz fpz;
	};
	union{
		int curPos;
		int mode;
	};
	int maxPos;
};

class vgmFile : public VgmFile_
{
public:
	vgx_Status status;
	void Save(const nchar* name, bool compress);
	char* Save(int& memSize);
};

class vgcFile : public VgcFile_
{
public:
	vgx_Status status;
	void Save(const nchar* name);
	char* Save(int& memSize);
};

class vgx
{
public:
	vgx();
	~vgx();
	void Free(void);
	int Open(const nchar* name);
	vgmFile* VgmFile(void);
	vgcFile* VgcFile(void);
	vgx_Status status;

private:
	void* file;
};

inline
vgmFile* vgx::VgmFile(void)
{
	return (vgmFile*)file;
}

inline
vgcFile* vgx::VgcFile(void)
{
	return (vgcFile*)file;
}

#endif
