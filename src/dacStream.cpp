#include "dacStream.hpp"

void dacStream::eventWrite(int targetSamp, int event, unsigned char *data)
{	
	if(current_samp != -1)
		dsOutput(targetSamp);
	codec::eventWrite(targetSamp, event, data);
}

void dacStream::Flush(int targetSamp)
{	
	if(current_samp != -1)
		dsOutput(targetSamp);
	codec::Flush(targetSamp);
}

void dacStream::dsOutput(int targetSamp)
{
	while(targetSamp >= (current_samp + int(remain_delay)))
	{
		current_samp += int(remain_delay);
		remain_delay -= int(remain_delay);
		remain_delay += sample_delay;
		codec::dacWrite(current_samp);
		//printf("%d, %d\n", remain_len, current_samp);
		remain_len -= 1;
		if(remain_len <= 0)
		{
			current_samp = -1;
			return;
		}
	}
}

bool dacStream::start(int targetSamp, int blockId,
	int seekPos, int mode, int length)
{	
	int blockCount = vgmInfo->dataBlock.blockCount;
	int seek;
	if(blockCount == 0)	{
		if(vgmInfo->sampData == NULL) return true;
		seek = 0; remain_len = vgmInfo->sampSize; }
	else { if(blockId >= blockCount) return true;
		seek = vgmInfo->dataBlock.blockList[blockId].sampData - vgmInfo->sampData;
		remain_len = vgmInfo->dataBlock.blockList[blockId].sampSize; }
		
	seek += seekPos; remain_len -= seekPos;
	if((mode != 1)&&(mode != 3)) return true;
	if((mode == 1)&&(remain_len > length))
		remain_len = length;
	if(remain_len < 0) return true;
	codec::eventWrite(targetSamp, EventSEEK,  (unsigned char*)&seek);
	remain_delay = sample_delay; current_samp = targetSamp;
	codec::dacWrite(current_samp); 	remain_len -= 1; return false;
}


bool dacStream::dsWrite(int targetSamp, unsigned char *data)
{
	if(current_samp != -1)
		dsOutput(targetSamp);

	switch(data[-1])
	{
		case 0x90:
		case 0x91:
			break;
		case 0x92:{
			// assume ym2612 stream
			long frequency = *(long*)&data[1];
			sample_delay = float(44100) / frequency;
			break;}
		case 0x94:
			// stop the stream
			current_samp = -1;
			break;
		case 0x93:
			return start(targetSamp, 0, *(int*)
				&data[1], data[5], *(int*)&data[6]);
		case 0x95:
			return start(targetSamp, *(unsigned
				short*)&data[1], 0, 3, 0);
	}
	return false;
}
