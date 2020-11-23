#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "stdshit.h"
#include "args.hpp"
#include "ret_codes.h"
#include "../vgmConv.h"

void print_help()
{
	printf("vgmConv: [options] <dest> <source>\n");
	printf("  -v: output format: vgm\n");
	printf("  -c: output format: vgc (default)\n");
	printf("  -dN: duplicate write removal mode\n");
	printf("  -i: init block enable\n");
	printf("  -z: vgz compression output\n");
	printf("  -q: quiet\n");
	printf("  -a: dac sample averge mode\n");
	printf("  -sN/D: rate scaling\n");
	printf("  -rSTR: rombuilder mode (STR optional)\n");
	printf("  -lN: rom size limit in kb (default: 4096)\n");
	printf("\nduplicate write remove modes\n");
	printf("  0: none (default)\n  1: safe\n  2: emulator\n  3: hardware\n");
}

int nmain(int argc, nchar *argv[])
{
	clock_t Time = clock();
	
	if(argc < 2) { 
		print_help(); 
		return 1; }
	
	// process arguments
	VgmConv_Args args;
	if( !args.getArgs(argc, argv))
	{
		if(args.silent == false)
			printf("%s\n", args.err_str);
		return return_map(error_badArgs + args.ret_code);
	}
	
	// get romfile name
	nchar* romFile = replName(
		argv[0], nstr("vgmPlay.dat"));

	// convert intput file
	int ret = vgmRom((_vgmConv*)&args, romFile);
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
