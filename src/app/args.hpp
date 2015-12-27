#ifndef _args_H_
#define _args_H_
#include "xplatform.h"
#include "../vgmConv.hpp"

struct VgmConv_Args : _vgmConv
{
	enum{
		args_bad,
		args_noDst,
		args_noSrc,
	};
	bool getArgs(int argc, nchar *argv[]);
	
	int ret_code;
	const char* err_str;
	bool silent;
};

#endif
