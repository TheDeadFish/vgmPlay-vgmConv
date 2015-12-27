// nothrow new/delete 
#include <stdlib.h>
#include <new>

void* __cxa_pure_virtual = 0;
namespace std{
	const nothrow_t nothrow = nothrow_t();
}

void *
operator new (size_t sz, const std::nothrow_t&)
{
	if (sz == 0)
		sz = 1;
	return malloc(sz);
}
void*
operator new[] (std::size_t sz, const std::nothrow_t& nothrow)
{
	return ::operator new(sz, nothrow);
}
void
operator delete(void* ptr)
{
	if(ptr)
		free(ptr);
}
void
operator delete[] (void *ptr, const std::nothrow_t&)
{
	::operator delete (ptr);
}
