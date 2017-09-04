#include "vgmDelay.hpp"

int vgmCodec_Delay(int targetSamp, int curSamp, MemWrite * outData)
{
	while(targetSamp > curSamp){
		int delay = targetSamp - curSamp;
		if(delay <= 16){
			// Output 0x7n delay
			outData->write8(0x70 + delay - 1);
			curSamp += delay;
			break;
		}
		switch(delay){
			case 1764:
				outData->write8(0x63);
			case 882:
				outData->write8(0x63);
				break;
			case 1470:
				outData->write8(0x62);
			case 735:
				outData->write8(0x62);
				break;
			default:
				if(delay > 0xffff)
				delay = 0xffff;
				outData->write8(0x61);
				outData->write16(delay);
				break;
		}
		curSamp += delay;
	}
	return(curSamp);
}
