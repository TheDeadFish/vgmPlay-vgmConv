// MemWrite library for VgmConv
// Uses setjump longjmp exception handling
#ifndef _MEMWRITE_H_
#define _MEMWRITE_H_
#include <stdlib.h>
#include <setjmp.h>

class MemWrite
{
public:
	MemWrite(){	
		Base = NULL;
	}
	
	~MemWrite(){
		Free();
	}
	
	void Free(void){
		if(Base)
		{
			free(Base);
			Base = NULL;
		}
	}
	
	void SetExcp(jmp_buf* jmpBuf, int code){
		this->jmpBuf = jmpBuf;
		this->excpCode = code;
	}
	
	char* getBase(void){
		return Base;
	}
	
	int curIndex(void){
		return CurPos - Base;
	}
	
	bool Alloc(int BlockSizeIn);
	int Array(char *buffer, int Size);

	template <class screb>
	int Write(screb a){
		if(sizeof(screb) <= 4){
			if(CurPos + sizeof(screb) >= CurMem){
				if(sizeof(screb) == 1) return WriteOVFB((char)a);
				if(sizeof(screb) == 2) return WriteOVFW((short)a);
				if(sizeof(screb) == 4) return WriteOVFL((int)a);
			}
			else{
				*(screb*)CurPos = a;
				CurPos += sizeof(screb);
			}
			return sizeof(screb);
		}
		return this->Array((char*)&a, sizeof(screb));
	}
	
	template <class screb>
	int WriteInd(screb *a){
				if(sizeof(screb) <= 4){
			if(CurPos + sizeof(screb) >= CurMem){
				if(sizeof(screb) == 1) return WriteOVFB((char)*a);
				if(sizeof(screb) == 2) return WriteOVFW((short)*a);
				if(sizeof(screb) == 4) return WriteOVFL((int)*a);
			}
			else{
				*(screb*)CurPos = *a;
				CurPos += sizeof(screb);
			}
			return sizeof(screb);
		}
		return this->Array((char*)a, sizeof(screb));
	}
	
	// implementation
private:
	void Grow(int size);
	int WriteOVFB(char a);
	int WriteOVFW(short a);
	int WriteOVFL(int a);
public:
	char *Base;
	char *CurPos;
	char *CurMem;
	int BlockSize;
	jmp_buf* jmpBuf;
	int excpCode;
};

#endif
