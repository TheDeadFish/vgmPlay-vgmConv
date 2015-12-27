#include "args.hpp"

bool VgmConv_Args::getArgs(int argc, nchar *argv[])
{
	// error strings
	static char err1[16] = "Unown option - ";
	static const char* err2 = "No source or destination specified";
	static const char* err3 = "No source file specified";
	
	dest = 0;
	source = 0;
	options = 0;
	silent = false;
	sscale = 1;
	dupRemove = 0;

	int step = 0;
	for(int i = 1; i < argc; i++){
		switch(step){
			case 0: // get option
				if(*argv[i] == '-'){
					switch(*(argv[i] + 1)){
						case 'v':
							options |= vgmFormat;
							continue;
						case 'c':
							options |= vgcFormat;
							continue;
						case 'd':
							dupRemove = argv[i][2]-'0';
							if(dupRemove > 3)
								goto INVALID_ARG;
							continue;
						case 'i':
							options |= writeInitB;
							continue;
						case 'z':
							options |= compressO;
							continue;
						case 'q':
							silent = true;
							continue;
						case 'a':
							options |= avSamp;
							continue;
						case 's':{
							double denomin = 1;
							_stscanf(argv[i]+2, _T("%lf/%lf"),
								&sscale, &denomin);
							sscale /= denomin;
							continue;}
						case '-':
							step++;
							continue;
						default:
						INVALID_ARG:
							err1[14] = (argv[i])[1];
							ret_code = args_bad;
							err_str = err1;
							return false;
					}
				}
				step++;
			case 1: // Get destination
				dest = argv[i];
				step++;
				continue;
			case 2: // Get source
				source = argv[i];
				break;
		}
		break;
	}
	if(dest == 0){
		ret_code = args_noDst;
		err_str = err2;
		return false;
	}
	if(source == 0){
		ret_code = args_noSrc;
		err_str = err3;
		return false;
	}
	// Make -c default; I don't like crashing
	if((options & vgmFormat) == 0)
		options |= vgcFormat;
	return true;
}
