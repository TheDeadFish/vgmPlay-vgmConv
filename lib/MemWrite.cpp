#include "MemWrite.hpp"

MemWrite* MemWrite::Grow(int size)
{
	// calculate indexes offsets
	int CurPosOffs = curIndex();
	int CurMemOffs = CurMem - Base; 
	int NewMemOffs = CurPosOffs + size;
	
	// calcxulate required size
	CurMemOffs += (CurMemOffs >> 1);
	if(CurMemOffs < NewMemOffs)
		CurMemOffs = NewMemOffs;
		
	// Reallocate
	void* tmp = realloc(Base, CurMemOffs);
	if(!tmp)
		longjmp(*jmpBuf, excpCode);
		
	// Update structure
	Base = (char*)tmp;
	CurPos = Base + CurPosOffs;
	CurMem = Base + CurMemOffs;		
	return this;
}

#define DEF_WRITE(n1,n2,t,s) \
	void MemWrite::n1(t v) { MemWrite* This = this; if(CurPos+s >= CurMem) \
		This = Grow(s); *(t*)(This->CurPos) = v; This->CurPos += s; } \
	void MemWrite::n2(void* p) { MemWrite* This = this; if(CurPos+s >= CurMem) \
		This = Grow(s); *(t*)(This->CurPos) = *(t*)p; This->CurPos += s; }
DEF_WRITE(write8,write8p,char,1);
DEF_WRITE(write16,write16p,short,2);
DEF_WRITE(write32,write32p,int,4);
