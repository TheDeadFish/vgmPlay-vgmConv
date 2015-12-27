#include "vgcDelay.hpp"
#include "vgcEvents.h"

int vgcCodec_Delay(int targetSamp, int curSamp, MemWrite *outData)
{
	while(targetSamp > curSamp){
		int delay = targetSamp - curSamp;
		if(delay <= 16){
			// Output 0x?n delay
			outData->Write<char>(VGC_N + delay - 1);
			curSamp += delay;
			break;
		}
		switch(delay){
			case 882:
				outData->Write<char>(VGC_50th);
				break;
			case 735:
				outData->Write<char>(VGC_60th);
				break;
			default:
				if(delay < 4096){
					// Output 0x?n nn delay
					outData->Write<char>(VGC_NNN + (delay & 0x0f));
					outData->Write<char>(delay >> 4);
				}else{
					if(delay > 0xffff)
						delay = 0xffff;
					// Output 0x?0 nn nn delay
					outData->Write<char>(VGC_NNNN);
					outData->Write<char>(delay >> 8);
					outData->Write<char>(delay & 255);
				}
		}
		curSamp += delay;
	}
	return(curSamp);
}
