#include "vgmDac.hpp"

void vgmDac::Init(int *curSampIn, MemWrite *outDataIn)
{
	curSamp = curSampIn;
	outData = outDataIn;
	writePend = false;
}

void vgmDac::Write(int targetSamp)
{
	if(writePend == false){
		*curSamp = vgmCodec_Delay(targetSamp, *curSamp, outData);
	}
	else{
		int delay = targetSamp - *curSamp;
		if(delay > 15)
			delay = 15;
		outData->write8(0x80 + delay);
		*curSamp += delay;
		*curSamp = vgmCodec_Delay(targetSamp, *curSamp, outData);
	}
	writePend = true;
}

void vgmDac::Flush(int targetSamp)
{
	if(writePend){
		int delay = targetSamp - *curSamp;
		if(delay > 15)
			delay = 15;
		outData->write8(0x80 + delay);
		*curSamp += delay;
	}
	*curSamp = vgmCodec_Delay(targetSamp, *curSamp, outData);
	writePend = false;
}

void *vgmDac::Get(int *size)
{
	*size = 0;
	return((void*)0);
}
