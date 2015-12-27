#include "vgcEvent.hpp"
#include "vgcEvents.h"

void vgcEvent::Init(int *curSampIn, MemWrite *outDataIn)
{
	curSamp = curSampIn;
	outData = outDataIn;
	eventPend = EventNULL;
}

void vgcEvent::Write(int targetSamp, int event, unsigned char *data)
{
	if(eventPend != EventNULL){
		int delay = targetSamp - *curSamp;
		if(delay > 15)
			delay = 15;
		switch(eventPend){
			case EventPSGP:
				//printf("%d, %d\n", eventPend, EventPSGP);
				//printf("%x, %x\n", (char)(0x20 + delay), *(char*)dataPend);
				outData->Write((char)(VGC_PSGP + delay));
				outData->WriteInd((char*)dataPend);
				break;
			case EventYMP0:
				outData->Write((char)(VGC_YMP0 + delay));
				outData->WriteInd((short*)dataPend);
				break;
			case EventYMP1:
				outData->Write((char)(VGC_YMP1 + delay));
				outData->WriteInd((short*)dataPend);
				break;
			case EventSEEK:
				outData->Write((char)(VGC_SEEK + delay));
				outData->WriteInd(dataPend + 2);
				outData->WriteInd(dataPend + 1);
				outData->WriteInd(dataPend + 0);
				break;
			case EventPSGS:
				outData->Write((char)(VGC_PSGS));
				outData->WriteInd((char*)dataPend);
				delay = 0;
				break;
			case Event_END:
				outData->Write((char)VGC_END);
				break;
		}
		*curSamp += delay;
	}
	if(targetSamp - *curSamp)
		*curSamp = vgcCodec_Delay(targetSamp, *curSamp, outData);
	
	eventPend = event;
	dataPend = data;
}

void vgcEvent::Flush(int targetSamp)
{
	Write(targetSamp, EventNULL, NULL);
}

void vgcEvent::State(int state)
{
}
