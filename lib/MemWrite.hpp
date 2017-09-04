// MemWrite library for VgmConv
// Uses setjump longjmp exception handling
#ifndef _MEMWRITE_H_
#define _MEMWRITE_H_
#include <stdlib.h>
#include <setjmp.h>

class MemWrite
{
public:
	MemWrite() : Base(0), 
		CurPos(0), CurMem(0) {}
	~MemWrite(){ free(Base); }
	void SetExcp(jmp_buf* jmpBuf, int code){
		this->jmpBuf = jmpBuf; this->excpCode = code; }
	char* getBase(void){ return Base; }
	int curIndex(void){ return CurPos - Base; }
		
	void write8(char); void write8p(void*);
	void write16(short); void write16p(void*);
	void write32(int); void write32p(void*);
	MemWrite* Grow(int);
	
	
	
private:

public:
	char *Base;
	char *CurPos;
	char *CurMem;

	jmp_buf* jmpBuf;
	int excpCode;
};

#endif
