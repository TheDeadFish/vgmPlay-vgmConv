#include "vgmEvent.hpp"

void vgmEvent::Init(int *curSampIn, MemWrite *outDataIn)
{
	curSamp = curSampIn;
	outData = outDataIn;
}

void vgmEvent::Write(int targetSamp, int event, unsigned char *data)
{
	
	// bring curSamp up the value of targetSamp
	*curSamp = vgmCodec_Delay(targetSamp, *curSamp, outData);

	// now curSamp is up to date, write event
	switch(event){	
		case EventPSGP:
			outData->write8(0x50);
			outData->write8p(data);
			break;
		case EventYMP0:
			outData->write8(0x52);
			outData->write16p(data);
			break;
		case EventYMP1:
			outData->write8(0x53);
			outData->write16p(data);
			break;
		case EventSEEK:
			outData->write8(0xe0);
			outData->write32p(data);
			break;
		case EventPSGS:
			outData->write8(0x4f);
			outData->write8p(data);
			break;
		case Event_END:
			outData->write8(0x66);
			break;
		//default:
			// This function does not handle error
	}
}

void vgmEvent::Flush(int targetSamp)
{
	*curSamp = vgmCodec_Delay(targetSamp, *curSamp, outData);
}

void vgmEvent::State(int state)
{
}
