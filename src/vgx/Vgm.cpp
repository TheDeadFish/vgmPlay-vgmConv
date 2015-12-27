#include "Vgm.hpp"
#include "Vgx.hpp"
#include <string.h>

int VgmFile_::ELen(unsigned char c)
{
	switch(c){
		case 0x62:
		case 0x63:
		case 0x66:
		case 0x70 ... 0x8f:
			return(0);
		case 0x30 ... 0x50:
			return(1);
		case 0x51 ... 0x61:
			return(2);
		case 0xc0 ... 0xdf:
			return(3);
		case 0xe0 ... 0xff:
			return(4);
		// detect V1.60 events
		case 0x68:
			return(11);
		case 0x90:
		case 0x91:
		case 0x95:
			return(4);
		case 0x92:
			return(5);
		case 0x93:
			return(10);
		case 0x94:
			return(1);
		default:
			return(-1);
	}
}

// vgm header
class vgm_header
{
public:
	int magic;
	int size;
	int version;
	int junk1[2];
	int gd3;
	int samples;
	int loop;
	int loopSamp;
	int junk2[4];
	int start;
	int junk3[2];
	
	vgm_header(){
		magic = 0x206D6756;
	}
	
	int getStart(void){
		return (start + 0x34);}
	
	void setStart(int in){
		start = in - 0x34;}
	
	int getLoop(void){
		return (loop + 0x1c);}
		
	void setLoop(int in){
		loop = in - 0x1c;}
		
	int getGd3(void){
		return (gd3 + 0x14);}
	
	void setGd3(int in){
		gd3 = in - 0x14;}
		
	int getSize(void){
		return (size + 4);}
	
	void setSize(int in){
		size = in - 4;}
		
	int getSamples(void){
		return samples;}
		
	void setSamples(int in){
		samples = in;}
		
	int getLoopSamp(void){
		return loopSamp;}
		
	void setLoopSamp(int in){
		loopSamp = in;}
};

bool VgmFile_::Load(vgx_fileIo& fp)
{
	// Error handling macros
	#define FP_READ(data, size){	\
		if(! fp.Read(data, size))	\
			return false;}
		
	#define FILE_BAD(){				\
		fp.FileBad();				\
		return false;}
	
	#define ALLOC_ERR(){			\
		fp.AllocErr();				\
		return false;}

		
	// Get header of vgm file
	vgm_header head;
	FP_READ(&head.size, sizeof(head) - 4);
	
	// Check vgm version number
	if(head.version > 0x00000160)
		FILE_BAD();
	
	// Allocate buffer for file
	int vgmSize = head.getSize();
	if(! buffer.alloc(vgmSize))
		ALLOC_ERR();
	header = (vgm_header*)buffer;
	
	// Move head to vgm buffer
	memcpy(header, &head, sizeof(head));

	// Read rest of vgm data
	int sizeRemain = vgmSize - sizeof(head);
	FP_READ(buffer + sizeof(head), sizeRemain);
	
	// Get pointer to start of vgm data
	if((head.version >= 0x00000150)&&(head.start != 0))
		mainData = buffer + head.getStart();
	else
		mainData = buffer + 0x40;
	
	// Get GD3 offset
	if(head.gd3 != 0)
		gd3Data = buffer + head.getGd3();
	else
		gd3Data = NULL;
	
	// Set data sizes
	if(gd3Data == NULL)
		mainSize = vgmSize - (mainData - buffer);
	else{
		if(gd3Data <= mainData)
			FILE_BAD();
			
		mainSize = gd3Data - mainData;
		gd3Size = vgmSize - (gd3Data - buffer);
	}
	
	// handle ambiguous starting point in pre V1.50 vgms
	if((mainSize >= 4)&&(*(long*)mainData == 0))
	{
		mainData += 4;
		mainSize -= 4;
	}
	
	// Get dac sample block
	switch( dataBlock.read( mainData, 
		mainSize, sampData, sampSize) )
	{
	case DataBlock::ERR_VGM:
		FILE_BAD();
	case DataBlock::ERR_ALLOC:
		ALLOC_ERR();
	}
		
	// Get loop index
	if(head.loop != 0){
		loopIndex = head.getLoop();
		loopIndex -= (mainData - buffer);
	}else
		loopIndex = 0xffffffff;
	return true;
}

bool VgmFile_::Save(vgx_fileIo& fp)
{
	// Error handling macros
	#define FP_WRITE(data, size){	\
		if(! fp.Write(data, size))	\
			return false;}
	
	// default header for when (header == NULL)
	vgm_header head;
	if(header == NULL)
	{
		memset(&head.size, 0, sizeof(head) - 4);
		head.version = 0x00000150;
		head.junk1[0] = 0x00369E99;
		head.junk2[1] = 0x00100009;
		head.junk2[2] = 0x00750AB5;
		header = &head;
	}
	
	// update header version
	if(dataBlock.blockCount)
	{
		if(header->version < 0x00000160)
			header->version = 0x00000160;	
	}
	
	// update header  VGM data offset
	if(header->version >= 0x00000150)
		header->start = 0xc;
	else
		header->start = 0x0;

	// calculate offset to vgm data
	int vgmOffs = 0x40;
	if(sampData != NULL){
		vgmOffs += sampSize;
		if(dataBlock.blockCount)
			vgmOffs += dataBlock.blockCount*7;
		else
			vgmOffs += 7;
	}
		
	// Calcluate offset to loopPoint
	if(loopIndex != 0xffffffff){
		int appsuluteIdx = vgmOffs + loopIndex;
		header->setLoop(appsuluteIdx);
	}else
		header->loop = 0;
		
	// update header GD3 offset
	vgmOffs += mainSize;
	if(gd3Data != NULL){
		header->setGd3(vgmOffs);
		vgmOffs += gd3Size;
	}
	
	// update remaining fields
	header->setSize(vgmOffs);
	header->setSamples(samples);
	header->setLoopSamp(loopSamp);
	
	// Reserve space in file
	if(! fp.Reserve(vgmOffs))
		return false;

	// Write vgm header to file
	FP_WRITE(header, sizeof(*header));
	
	// Write dac sample data to file
	if(sampData != NULL){
		char sampHead[7] = {0x67, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00};
		// check for dac streams
		if(dataBlock.blockCount)
		{
			for(int i = 0; i < dataBlock.blockCount; i++)
			{
				*(int*)&sampHead[3] = dataBlock.blockList[i].sampSize;
				FP_WRITE(sampHead, 7);
				FP_WRITE(dataBlock.blockList[i].sampData, 
					dataBlock.blockList[i].sampSize);
			}
		}
		else
		{
			*(int*)&sampHead[3] = sampSize;
			FP_WRITE(sampHead, 7);
			FP_WRITE(sampData, sampSize);
		}
	}
	
	// Write main vgm data
	FP_WRITE(mainData, mainSize);
	
	// Write gd3 data
	if(gd3Data != NULL)
		FP_WRITE(gd3Data, gd3Size);
	return true;
}
