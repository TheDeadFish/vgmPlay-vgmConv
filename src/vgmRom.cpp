#include "stdshit.h"
#include "vgmConv.hpp"
#include "MemWrite.hpp"

int vgcFile_setSize(void* ptr, int size);

void fileError(nchar* str) 
{
	_tprintf(nstr("error: %s\n"), str);
}

struct VgmRom
{
	_vgmConv* in_out;
	
	int build(nchar* progName);
private:
	
	int add_file(nchar* str);
	int load_m3u(nchar* str);
	int rom_init();
	int rom_flush();
	
	AutoMem<char> rom_data; size_t rom_size;
	AutoMem<char> vgc_data; size_t vgc_size;
	AutoMem<char> end_data; int end_size;
	
	int rom_count; int track_count;
};

int VgmRom::load_m3u(nchar* fName)
{
	// open m3u file
	CFile fp(nfopen(fName, nstr("r")));
	if(!fp) { fileError(fName); return vgmConv_srcErr; }
		
	// loop over lines
	nchar line[260];
	while(_fgetts(line, 260, fp)) { 
		removeCrap(line); 
		AutoMem<nchar> name(replName(fName, line));
		if(!name) return vgmConv_MemErr;
		IFRET(add_file(name));
	}
	return 0;
}

int VgmRom::rom_init()
{
	vgcFile vgcInfo = {};
	
	
	nchar name[1024];
	_stprintf(name, in_out->dest, rom_count);
	vgcInfo.gd3Data = (char*)name;
	vgcInfo.gd3Size = strsize(name);
	vgcInfo.extraData = (char*)in_out->romName;
	vgcInfo.extraSize = strsize(in_out->romName);
	
	end_data.data = vgcInfo.Save(end_size);
	return vgcInfo.status;
}

int VgmRom::rom_flush()
{
	if(!vgc_data) return 0;

	// open output file
	nchar name[1024];
	_stprintf(name, in_out->dest, rom_count++);
	CFile fp(nfopen(name, nstr("wb")));
	if(!fp) { fileError(name); return vgmConv_dstErr; }

	// write vgmPlay
	if(!fp.write(rom_data, rom_size))
		return vgmConv_WritErr;	

	// write vgc data
	if(!fp.write(vgc_data, vgc_size))
		return vgmConv_WritErr;
		
	if((end_data)
	&&(!fp.write(end_data, end_size)))
		return vgmConv_WritErr;
		
	// reset state
	vgc_size = 0;
	vgc_data.free();
	end_data.free();	
		
	return rom_init();
}

int VgmRom::add_file(nchar* fName)
{
	printf("%S\n", fName);

	// encode vgc file
	_vgmConv tmp = *in_out;
	tmp.source = fName; tmp.dest = NULL;
	tmp.romName = fName;
	int ret = vgmConv(&tmp);
	if(ret) { fileError(fName);	return ret; }
	
	// limit check
	size_t tmpSize = rom_size + vgc_size + end_size + tmp.fileSize;
	if(tmpSize > tmp.romLimit) { 
		if((ret = rom_flush())) goto ERRET; }

	// append vgc data
	if(vgc_data.resize(vgc_size+tmp.fileSize)) {
		memcpy(vgc_data+vgc_size, tmp.fileData, tmp.fileSize);
		vgc_size += tmp.fileSize; track_count++;
	} else { ret = vgmConv_MemErr; }

ERRET:
	free(tmp.fileData);
	return ret;
}

int VgmRom::build(nchar* romFile)
{
	IFRET(rom_init());

	// load rom file
	rom_data.data = load_file(romFile, rom_size);
	if(!rom_data) { fileError(romFile);
		return vgmConv_srcErr; }

	// read m3u file
	int ret = load_m3u(in_out->source);
	if(ret) return ret;
	return rom_flush();
}

int vgmRom(_vgmConv* in_out, nchar* romFile)
{
	if(!in_out->romName)
		return vgmConv(in_out);
	VgmRom vgmRom = {};
	vgmRom.in_out = in_out;
	return vgmRom.build(romFile);
}
