#include "stdshit.h"
#include "vgx/vgmEvent.hpp"


void vgmEvents_dedup(VgmEvents& events)
{

#if 0
	VgmEvent* dacSeek[2] = {};
	VgmEvent* dacWrite = NULL;
	int dacPos = -1;
	
	VgmEventFrame frame;
	for(auto& event : events) {
		if(events.isLoop(event)) dacPos = -1;
	
		if(frame.check(event)) {
		
		
		
			
		
		
		
		
		
		
		}
		
		
		if(event) {
		
			switch(event[0]) {
			case 0x80 ... 0x8F:
				if(!dacWrite) {
					dacWrite = &event;
					if(
				
				
				
				
				}
			
			
			
			case 0xE0:
				if(!dacWrite) {
					dacSeek[0]->data = NULL;
					dacSeek[0] = &event;
				}
				
				
				dacWrite = NULL;
				
			
				
			
			
				
			
			
			
			
			}
		
		
		
		}
		
		
	}
	
	
	









#endif










}
