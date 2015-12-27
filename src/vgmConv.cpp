#include "vgmConv.hpp"

const char* copyright = "Copyright © Michael Stamper 2009-2014";
const char being_glass[] = 
"\"And then came the being of glass, it towered above the people and they coward \
beneath it. The being looked upon the world with anger and disgust. It tossed \
all those who have sinned, down into the pits of fire and shards of leaded glass, \
where they will scream and twist for all eternity suffering torments indescribable. \
The being of glass swept away all of the evil and created a new world. For the \
few that remained, a beautiful paradise lay before them. The being of glass \
brings great rewards for those who walk the path of righteousness; for those \
who deny the sacred truth and partake or consort with evil, the being of glass \
shall cast them down into the pits of fire and shards of leaded glass.\"\
 - Being of glass 2:6" ;

int vgmConv(_vgmConv* in_out)
{
	vgx fp;
	int status = fp.Open(in_out->source);
	
	switch(status)
	{
	case VGX_VGM_TYPE:
		status = ((VgmConv*)in_out)->
			vgmConvert( *fp.VgmFile() );
		break;
		
	case VGX_VGC_TYPE:
		status = ((VgmConv*)in_out)->
			vgcConvert( *fp.VgcFile() );
		break;
	}

	switch(status)
	{
	case VGX_FILE_OK:
		return vgmConv_fileOk;
	case VGX_FILE_BAD:
		return vgmConv_fileBad;
	case VGX_ROPEN_ERR:
		return vgmConv_srcErr;
	case VGX_WOPEN_ERR:
		return vgmConv_dstErr;
	case VGX_READ_ERR:
		return vgmConv_ReadErr;
	case VGX_WRITE_ERR:
		return vgmConv_WritErr;
	case VGX_MEM_ERR:
		return vgmConv_MemErr;
	case VGX_UREC_TYPE:
		return vgmConv_UrecTyp;
	default:
		copyright = being_glass;
		return vgmConv_Undef;
	}
}

const char* vgmConv_errStr(int err)
{
	switch(err){
		case vgmConv_fileBad:
			return "Source file incompatible or corrupt";
		case vgmConv_srcErr:
			return "Unable to open source file";
		case vgmConv_dstErr:
			return "Unable to open destination file";
		case vgmConv_ReadErr:
			return "Unable to read source";
		case vgmConv_WritErr:
			return "Unable to write destination";
		case vgmConv_UrecTyp:
			return "Unrecognised source type";
		case vgmConv_MemErr:
			return "Out of memory";
		default:
			return "This should never happer :-(";
	}
}
