#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "xplatform.h"
#include "args.hpp"
#include "ret_codes.h"
#include "../vgmConv.h"

int nmain(int argc, nchar *argv[])
{
	clock_t Time = clock();
	
	// process arguments
	VgmConv_Args args;
	if( !args.getArgs(argc, argv))
	{
		if(args.silent == false)
			printf("%s\n", args.err_str);
		return return_map(error_badArgs + args.ret_code);
	}
	
	// convert intput file
	int ret = vgmConv((_vgmConv*)&args);
	if(ret)
	{
		if(args.silent == false)
			printf("%s\n", vgmConv_errStr(ret));
		if(ret == vgmConv_Undef)
			return return_map(error_undef);
		return return_map(
			error_fileBad + ret-1);
	}
	
	// display time taken
	if(args.silent == false){
		Time = clock() - Time;
		printf("processed in %.02fs\n", (float)Time / CLOCKS_PER_SEC);
	}
	return return_map(error_success);
}
