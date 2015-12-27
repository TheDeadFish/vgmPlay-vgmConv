#include "vgmDelay.hpp"

int vgmCodec_Delay(int targetSamp, int curSamp, MemWrite * outData)
{
	while(targetSamp > curSamp){
		int delay = targetSamp - curSamp;
		if(delay <= 16){
			// Output 0x7n delay
			outData->Write<char>(0x70 + delay - 1);
			curSamp += delay;
			break;
		}
		switch(delay){
			case 1764:
				outData->Write<char>(0x63);
			case 882:
				outData->Write<char>(0x63);
				break;
			case 1470:
				outData->Write<char>(0x62);
			case 735:
				outData->Write<char>(0x62);
				break;
			default:
				if(delay > 0xffff)
				delay = 0xffff;
				outData->Write<char>(0x61);
				outData->Write<unsigned short>(delay);
				break;
		}
		curSamp += delay;
	}
	return(curSamp);
}
