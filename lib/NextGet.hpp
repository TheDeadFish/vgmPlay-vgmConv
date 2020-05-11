// Binary file parser with setjmp exceptions
// Deadfish Shitware 2012
#ifndef _NEXTGET_H_
#define _NEXTGET_H_
#include <setjmp.h>
#include <stddef.h>

// Common members
class NextGet_
{
public:
	char* curPos;
	template <typename screb>
	void Add(screb a){
		curPos += a;
	}
	template <typename screb>
	void Sub(screb a){
		curPos -= a;
	}
	template <typename screb>
	unsigned int Idx(screb *a){
		return(curPos - a);
	}
};

// Exception Range Checked
class NextGet : public NextGet_
{
public:
	// Constructor / Init
	NextGet(){}
	template <typename T>
	NextGet(T* data, size_t size,
		jmp_buf& jmpBuf, int code){
		SetData(data, size);
		SetExcp(jmpBuf, code);
	}
	template <typename T>
	void SetData(T* data, int size){
		curPos = (char*)data;
		maxPos = curPos + size;
	}
	void SetExcp(jmp_buf& jmpBuf, int code){
		this->jmpBuf = &jmpBuf;
		this->excpCode = code;
	}

	// Range checked operations
	template <typename T>
	T Get()
	{
		T tmp;
		if(curPos + sizeof(T) > maxPos)
			Error();
		else
		{
			tmp = *(T*)curPos;
			curPos += sizeof(T);
			return(tmp);
		}
	}
	template <typename T>
	void Range(T a){
		if(curPos + a > maxPos)
			Error();
	}

	char *maxPos;
	jmp_buf* jmpBuf;
	int excpCode;
	[[ noreturn ]] void Error(void)
	{	longjmp(*jmpBuf, excpCode);	}
};

// Unchecked  
class NxtGet : public NextGet_
{
public:
	// Constructor / Init
	NxtGet(){}
	template <typename T>
	NxtGet(T* data){
		SetData(data);}
	template <typename T>
	void SetData(T* data){
		curPos = (char*)data;}

	template <typename T>
	T Get(){
		T tmp;
		tmp = *(T*)curPos;
		curPos += sizeof(T);
		return(tmp);
	}
};

#endif

