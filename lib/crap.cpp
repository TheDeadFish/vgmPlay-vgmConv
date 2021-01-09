#include "stdshit.h"
#include "ccstuff.h"
#include <errno.h>
#include <stdarg.h>

void AutoMem_Free(void* pHack)
{
	void** data = (void**)pHack;
	void* tmp = *data; *data = 0;
	if(tmp) free(tmp);
}

int fopen_ErrChk(void)
{
	switch(errno)
	{
	case ENOENT:
	case EACCES:
	case EISDIR:
	case EINVAL:
		return 1;
	default:
		return 0;
	}
}

void CFile::close(void)
{
	if(fp) {
		fclose(fp);
		fp = NULL;
	}
}

size_t CFile::fSize()
{
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

bool CFile::read(void* data, size_t size)
{
	return fread(data, size, 1, fp);
}

bool CFile::write(const void* data, size_t size)
{
	return fwrite(data, size, 1, fp);
}

char* load_file(nchar* name, size_t& size)
{
	CFile fp(nfopen(name, nstr("rb")));
	if(!fp) return NULL;
	size = fp.fSize();
	char* data = (char*)malloc(size);
	if(data && !fp.read(data, size)) {
		free(data); data = NULL; }
	return data;
}

int removeCrap(nchar* str)
{
	int len = _tstrlen(str);
	while(len--)
		if(uns(str[len]) > ' ')
			break;
	str[len+1] = '\0';
	return len+1;
}

int getPathLen(const nchar* name)
{
	int pathLen = 0;
	for(int i = 0; name[i]; i++) {
		if(isSep(name[i])) pathLen = i+1; }
	return pathLen;
}

int strsize(const nchar* str)
{
	if(!str) return 0;
	int len = _tstrlen(str);
	return (len+1)*sizeof(nchar);
}

nchar* replName(const nchar* path, const nchar* name)
{
	int size = getPathLen(path) * sizeof(nchar);
	char* buff = (char*)malloc(strsize(name) + size);
	if(buff) { memcpy(buff, path, size);
		_tcscpy((nchar*)(buff+size), name); }
	
	return (nchar*)buff;
}

__thiscall
void* xrealloc(void* ptr, size_t size) {
	void* tmp = realloc(*(void**)ptr, size);
	*(void**)ptr = tmp; return tmp; }

__thiscall
void* xNextAlloc_(void* ptr, uint& count, uint size)
{
	uint tmp = count; count++;
	char* base;

	if(!(tmp & (tmp-1))) {
		uint xxx = tmp; xxx *= 2; if(!xxx) xxx = 1;
		base = (char*)xrealloc(ptr, xxx*size);
	} else { base = *(char**)ptr; }
	return  base + tmp*size;
}


// error handling
void fatal_error(const char*fmt,...) {
	va_list args; va_start (args, fmt);
	vfprintf(stderr, fmt,args);
	exit(1); va_end (args); }
#define FATAL_ERROR(str, arg, fileName) \
	fatal_error(str ", \"%s\"\n", arg, fileName);
void load_error(const char* type, const char* fileName) {
	FATAL_ERROR("%s: open failed", type, fileName); }
void file_corrupt(const char* type, const char* fileName) {
	FATAL_ERROR("%s: file corrupt", type, fileName); }
void file_bad(const char* type, const char* fileName) {
	FATAL_ERROR("%s: unsupported file data", type, fileName); }
void recurse_error(const char* type) {
	fatal_error("%s: infinite recursion error", type); }
	
#define error_msg(fmt, ...) \
	fprintf(stderr, fmt, __VA_ARGS__);
