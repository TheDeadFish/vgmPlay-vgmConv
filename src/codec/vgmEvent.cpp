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
			outData->Write((char)0x50);
			outData->WriteInd((char*)data);
			break;
		case EventYMP0:
			outData->Write((char)0x52);
			outData->WriteInd((short*)data);
			break;
		case EventYMP1:
			outData->Write((char)0x53);
			outData->WriteInd((short*)data);
			break;
		case EventSEEK:
			outData->Write((char)0xe0);
			outData->WriteInd((long*)data);
			break;
		case EventPSGS:
			outData->Write((char)0x4f);
			outData->WriteInd((char*)data);
			break;
		case Event_END:
			outData->Write((char)0x66);
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
