// vgm multiple data block parser
#ifndef INC_DATABLOCK_H_
#define INC_DATABLOCK_H_
#include "ccstuff.h"

class DataBlock
{
public:
	DataBlock();
	void free(void);
	int read(
		char*& mainData, int& mainSize,
		char*& sampData, int& sampSize
	);
	
	enum{
		ERR_NONE,
		ERR_VGM,
		ERR_ALLOC
	};
	
	struct BlockList
	{
		char* sampData;
		int sampSize;
	};
	int blockCount;
	AutoMem<BlockList> blockList;
};


inline
DataBlock::DataBlock()
{
	blockCount = 0;
}

#endif
