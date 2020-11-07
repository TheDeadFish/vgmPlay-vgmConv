// C++ Wrappers etc
// DeadFish Software, 2012
#ifndef _CCSTUFF_H_
#define _CCSTUFF_H_
#include <stdio.h>
#include <stdlib.h>

// AutoMem
template <class T>
class AutoMem
{
public:
	T* data;
	AutoMem();
	~AutoMem();
	void free(void);
	T* release(void);
	bool alloc(size_t size);
	bool resize(size_t size);
	operator T*();
	template <class U>
	explicit operator U();
};

// AutoMem: Implementation
void AutoMem_Free(void* pHack);

template <class T>
AutoMem<T>::AutoMem()
{
	data = 0;
}

template <class T>
AutoMem<T>::~AutoMem()
{
	this->free();
}

template <class T>
void AutoMem<T>::free(void)
{
	AutoMem_Free(this);
}

template <class T>
T* AutoMem<T>::release(void)
{
	T* tmp = data;
	data = 0;
	return tmp;
}

template <class T>
bool AutoMem<T>::alloc(size_t size)
{
	this->free();
	data = (T*)malloc(size * sizeof(T));
	return data;
}

template <class T>
bool AutoMem<T>::resize(size_t size)
{
	T* tmp = (T*)realloc(data, size * sizeof(T));
	if(tmp == 0)
	{
		this->free();
		return false;
	}
	data = tmp;
	return true;
}

template <class T>
AutoMem<T>::operator T*()
{
	return data;
}

template <class T>
template <class U>
AutoMem<T>::operator U()
{
	return (U)data;
}

struct CFile
{
	FILE* fp;
	
	~CFile() { close(); } 
	CFile(FILE* fp_ = 0) : fp(fp_) {}
	
	operator FILE*() { return fp; }
	
	
	void close();
	size_t fSize();
	
	bool read(void* data, size_t size);
	bool write(const void* data, size_t size);
};

#endif
