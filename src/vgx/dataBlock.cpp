#include "dataBlock.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define dataBlock_allocSize 16

void DataBlock::free()
{
	blockList.free();
	blockCount = 0;
}

int DataBlock::read(
	char*& mainData, int& mainSize, 
	char*& sampData, int& sampSize
){
	this->free();
	sampData = NULL;
	while(mainData[0] == 0x67)
	{
		// check dataBlock format
		int size;
		if(mainSize <= 7)
			return ERR_VGM;
		if(*(short*)(&mainData[1]) != 0x66)
			return ERR_VGM;
		size = *(unsigned long*)&mainData[3];
		if(mainSize <= size + 7)
			return(ERR_VGM);
		
		// check allocation size
		if(blockCount == 1)
		{
			if(! blockList.alloc(dataBlock_allocSize))
				return ERR_ALLOC;
			blockList[0].sampData = sampData;
			blockList[0].sampSize = sampSize;
		}
		if(blockCount > 1)
		{
			if((blockCount % dataBlock_allocSize) == 0)
			{
				if(! blockList.resize(blockCount + dataBlock_allocSize))
					return ERR_ALLOC;
			}
		}
	
		// add current block to list
		if(blockCount == 0)
		{
			sampData = mainData + 7;
			sampSize = size;
		}
		else
		{
			memmove(mainData, mainData + 7, size);
			blockList[blockCount].sampData = mainData;
			blockList[blockCount].sampSize = size;
			sampSize += size;
		}
		blockCount++;

		// Remove dac samples from vgm data
		mainData += (size + 7);
		mainSize -= (size + 7);
	}
	if(blockCount == 1)
		blockCount = 0;
	return ERR_NONE;
}
