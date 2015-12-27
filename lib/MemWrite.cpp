#include "MemWrite.hpp"
#include <new>

bool MemWrite::Alloc(int BlockSizeIn)
{
	Free();
	void* tmp = malloc(BlockSizeIn);
	if(!tmp)
		return false;

	// Update structure
	Base = (char*)tmp;
	CurPos = Base + 0;
	CurMem = Base + BlockSizeIn;
	BlockSize = BlockSizeIn;
	return true;
}

void MemWrite::Grow(int size)
{
	// Create local aposolute data
	int CurPosOffs = curIndex();
	int CurMemOffs = CurMem - Base; 
	int AllocSize = CurMemOffs;

	// Calculate required size
	do{
		AllocSize += BlockSize;
	}while(AllocSize <= (CurPosOffs + size));
	
	// Reallocate
	void* tmp = realloc(Base, AllocSize);
	if(!tmp)
		longjmp(*jmpBuf, excpCode);
		
	// Update structure
	Base = (char*)tmp;
	CurPos = Base + CurPosOffs;
	CurMem = Base + AllocSize;
}

int MemWrite::Array(char *buffer, int Size)
{
	if(CurMem <= (CurPos + Size))
		this->Grow(Size);
	// Memory has now been grown enough
	asm volatile (
	"	mov %3,%%ecx			\n"
	"	shrl %%ecx				\n"
	"	shrl %%ecx				\n"
	"	rep movsl				\n"
	"	mov	%3,%%ecx			\n"
	"	and $3,%%ecx			\n"
	"	rep movsb				\n"
	: "=D"(CurPos)
	: "S"(buffer), "D"(CurPos), "g"(Size)
	: "ecx");
	return(Size);
}

int MemWrite::WriteOVFB(char a)
{
	this->Grow(1);
	*CurPos++ = a;
	return(1);
}

int MemWrite::WriteOVFW(short a)
{
	this->Grow(2);
	*(short*)CurPos = a;
	return(2);
}

int MemWrite::WriteOVFL(int a)
{
	this->Grow(4);
	*(int*)CurPos = a;
	CurPos += 4;
	return(4);
}
