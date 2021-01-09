#include "stdshit.h"
#include "Vgm.hpp"
#include "VgmEvent.hpp"
#include "VgmLen.h"

bool VgmEvents::init(VgmFile_& vgmInfo)
{
	byte* curPos = (byte*)vgmInfo.mainData;
	byte* endPos = curPos+vgmInfo.mainSize;
	byte* loopPos = (byte*)vgmInfo.getLoop();
	int curSample = 0;
	initSample = -1;
	int nLoop = -1;
	
	while(1) {
		if(curPos >= endPos) return false;
		char len = vgmLen[*curPos];
		if(!len) return false;
		
		if((curPos == loopPos)||(len < 0)) {
			if(curPos == loopPos) nLoop = nEvents; 
			auto& event = xNextAlloc(events, nEvents);
			event = {curSample, NULL};
			if(len < 0) {	event.data = curPos;
				if(initSample < 0) initSample = curSample; }
		}
	
		switch(*curPos) {
		case 0x66:
			if(nLoop >= 0)
				this->loopPos = events+nLoop;
			return true;
		case 0x70 ... 0x7F:
			curSample += 1;
		case 0x80 ... 0x8F:
			curSample += *curPos & 0x0F;
			break;
		case 0x61:
			curSample += *(u16*)(curPos+1);
			break;
		case 0x62:
			curSample += 735;
			break;
		case 0x63:
			curSample += 882;
			break;
		}
		
		curPos += len & 0x7f;
	}
	
	return false;
}
