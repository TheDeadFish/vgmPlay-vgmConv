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
				outData->write8((VGC_PSGP + delay));
				outData->write8p(dataPend);
				break;
			case EventYMP0:
				outData->write8((VGC_YMP0 + delay));
				outData->write16p(dataPend);
				break;
			case EventYMP1:
				outData->write8((VGC_YMP1 + delay));
				outData->write16p(dataPend);
				break;
			case EventSEEK:
				outData->write8((VGC_SEEK + delay));
				outData->write8p(dataPend + 2);
				outData->write8p(dataPend + 1);
				outData->write8p(dataPend + 0);
				break;
			case EventPSGS:
				outData->write8((VGC_PSGS));
				outData->write8p(dataPend);
				delay = 0;
				break;
			case Event_END:
				outData->write8(VGC_END);
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
