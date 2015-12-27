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

bool dacStream::dsWrite(int targetSamp, unsigned char *data)
{
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
		case 0x95:{
			// and now for some real meat
			int blockId = *(unsigned short*)&data[1];
			int blockCount = vgmInfo->dataBlock.blockCount;
			int seek;
			if(blockCount == 0)
			{
				if(vgmInfo->sampData == NULL)
					return true;
				seek = 0;
				remain_len = vgmInfo->sampSize;
			}
			else
			{
				if(blockId >= blockCount)
					return true;
				seek = vgmInfo->dataBlock.blockList[blockId].sampData - vgmInfo->sampData;
				remain_len = vgmInfo->dataBlock.blockList[blockId].sampSize;
			}
			//printf("0x95 %d\n", targetSamp);
			codec::eventWrite(targetSamp, EventSEEK,  (unsigned char*)&seek);
			// initialise stream state
			remain_delay = sample_delay;
			current_samp = targetSamp;
			// write first sample
			codec::dacWrite(current_samp);
			remain_len -= 1;
			break;}
	}
	return false;
}
