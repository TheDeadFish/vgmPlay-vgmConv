#include "ccstuff.h"
#include "xplatform.h"
#include <errno.h>

void AutoMem_Free(void* pHack)
{
	void* data = *(void**)pHack;
	if(data)
		free(data);
	data = 0;
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
