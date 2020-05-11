#include "Vgx.hpp"
#include <stdlib.h>
#include <string.h>
#include <new>
using namespace std;

// file io wrapper
vgx_fileIo::vgx_fileIo()
{
	fp = NULL;
	poit = NULL;
	maxPos = -1;
}

vgx_fileIo::~vgx_fileIo()
{
	Close();
}

void vgx_fileIo::Close(void)
{
	if(maxPos == -1){
		#ifdef HAS_ZLIB
			if(mode == wb){
				if(fp != NULL)
					fclose(fp);
			}else{
				if(fpz != NULL)
					gzclose(fpz);
			}
		#else
			if(fp != NULL)
				fclose(fp);
		#endif
	}else{
		if(poit != NULL)
			free(poit);
	}
	fp = NULL;
	poit = NULL;
	maxPos = -1;
}

bool vgx_fileIo::Open(const nchar* name, char _mode)
{
	Close();
	status = VGX_FILE_OK;
	
	// check for memory write
	if(_mode == mw){
		curPos = 0;
		maxPos = 0;
		return true;
	}
	
	// open the file
	const nchar* mode_str[] = {nstr("rb"),
		nstr("wb"), nstr("wb")};
	fp = nfopen(name, mode_str[_mode]);
	if(fp == NULL)
	{
		if( fopen_ErrChk() )
		{
			if(_mode == rb)
				status = VGX_ROPEN_ERR;
			else
				status = VGX_WOPEN_ERR;
		}
		else
		{
			status = VGX_MEM_ERR;
		}
		return false;
	}
		
	// open with zlib
	#ifdef HAS_ZLIB
		mode = _mode;
		if(_mode == wb)
			return true;
		const char* mode_str2[] = {"rb", "wb0", "wb9"};
		gzFile tmp = gzdopen(fileno(fp), mode_str2[_mode]);
		if(tmp == NULL)
		{
			fclose(fp);
			status = VGX_MEM_ERR;
			return false;
		}
		fpz = tmp;
	#endif
	return true;
}

bool vgx_fileIo::Read(void* buff, int len)
{
	status = VGX_FILE_OK;
	#ifdef HAS_ZLIB
		// zlib read
		if(gzread(fpz, buff, len) != len)
		{
			int error;
			gzerror(fpz, &error);
			if(error == Z_ERRNO)
				status = VGX_READ_ERR;
			else
				status = VGX_FILE_BAD;
			return false;
		}
	#else
		// stdio read
		if(fread(buff, len, 1, fp) != 1)
		{
			if(feof(fp) == 0)
				status = VGX_READ_ERR;
			else
				status = VGX_FILE_BAD;
			return false;
		}
	#endif
	return true;
}

bool vgx_fileIo::Write(void* buff, int len)
{
	status = VGX_FILE_OK;
	
	// check for memory write
	if(maxPos != -1){
		if(Reserve(curPos + len));
		{
			memcpy(&poit[curPos], buff, len);
			curPos += len;
			return true;
		}
		return false;
	}
	
	#ifdef HAS_ZLIB
		// zlib write
		if(mode == wb){
			if(fwrite(buff, len, 1, fp) != 1)
			{
				status = VGX_WRITE_ERR;
				return false;
			}
		}else{
			if(gzwrite(fpz, buff, len) != len)
			{
				status = VGX_WRITE_ERR;
				return false;
			}
		}
	#else
		// stdio write
		if(fwrite(buff, len, 1, fp) != 1)
		{
			status = VGX_WRITE_ERR;
			return false;
		}
	#endif
	return true;
}

bool vgx_fileIo::Reserve(int reqSize)
{
	status = VGX_FILE_OK;
	if(maxPos == -1)
		return true;
	if(maxPos >= reqSize)
		return true;
	// reallocate
	void* tmp = realloc(poit, reqSize);
	if(tmp == NULL)
	{
		status = VGX_MEM_ERR;
		return false;
	}
	poit = (char*)tmp;
	maxPos = reqSize;
	return true;
}

char* vgx_fileIo::Release(int& memSize)
{
	memSize = curPos;
	char* ret = poit;
	poit = NULL;
	return ret;
}

void vgx_fileIo::FileBad(void)
{
	status = VGX_FILE_BAD;
}

void vgx_fileIo::AllocErr(void)
{
	status = VGX_MEM_ERR;
}


// vgmFile, vgcFile completion
void vgmFile::Save(const nchar* name, bool compress)
{
	// open file for write
	vgx_fileIo fp;
	if( fp.Open(name, (compress) ? fp.wbz : fp.wb) )
	{
		// Write vgm file
		VgmFile_::Save(fp);
	}
	status = fp.status;
}

char* vgmFile::Save(int& memSize)
{
	// open memory for write
	vgx_fileIo fp;
	if( fp.Open(NULL, fp.mw) )
	{
		// Write vgm file
		if( VgmFile_::Save(fp))
		{
			// Release memory
			status = fp.status;
			return fp.Release(memSize);
		}
	}
	status = fp.status;
	return NULL;
}

void vgcFile::Save(const nchar* name)
{
	// open file for write
	vgx_fileIo fp;
	if( fp.Open(name, fp.wb) )
	{
		// Write vgm file
		VgcFile_::Save(fp);
	}
	status = fp.status;
}

char* vgcFile::Save(int& memSize)
{
	// open memory for write
	vgx_fileIo fp;
	if( fp.Open(NULL, fp.mw) )
	{
		// Write vgm file
		if( VgcFile_::Save(fp))
		{
			// Release memory
			status = fp.status;
			return fp.Release(memSize);
		}
	}
	status = fp.status;
	return NULL;
}


// The vgx class itself
vgx::vgx()
{
	status = VGX_UREC_TYPE;
	file = NULL;
}

vgx::~vgx()
{
	Free();
}

void vgx::Free(void)
{
	switch(status){
		case VGX_VGM_TYPE:
			delete VgmFile();
			break;
		case VGX_VGC_TYPE:
			delete VgcFile();
			break;
	}
	status = VGX_UREC_TYPE;
	file = NULL;
}

int vgx::Open(const nchar* name)
{
	#define ERR_RETURN(err){	\
		status = err;			\
		return status;}
		
	// open vgm file
	Free();
	vgx_fileIo fp;
	if(! fp.Open(name, fp.rb))
		ERR_RETURN(fp.status);
	
	// check format
	int magic;
	if(! fp.Read(&magic, 4))
		ERR_RETURN(fp.status);

	switch(magic){
		case 0x206D6756:
			file = new(nothrow) vgmFile();
			if(file == 0)
				ERR_RETURN(VGX_MEM_ERR);
			status = VGX_VGM_TYPE;
			
			if(! VgmFile()->Load(fp))
			{
				status = VgmFile()->status;
				this->Free();
				return status;
			}
			return status;
			
		case 0x20636756:
			file = new(nothrow) vgcFile();
			if(file == 0)
				ERR_RETURN(VGX_MEM_ERR);
			status = VGX_VGC_TYPE;
			
			if(! VgcFile()->Load(fp))
			{
				status = VgcFile()->status;
				this->Free();
				return status;
			}
			return status;
		
		default:
			status = VGX_UREC_TYPE;
			return status;
	}
}
