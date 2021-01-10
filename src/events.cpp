#include "stdshit.h"
#include "vgx/vgmEvent.hpp"
#include <windows.h>

void filter_dac(VgmEvents& events)
{
	VgmEvent* lastWrite = NULL;
	VgmEvent* writeSeek = NULL;
	VgmEvent* lastSeek = NULL;
	
	VGMEVENT_FILTER(events,,
	
		if(lastWrite && writeSeek) {
			
			for(; prevSampPos < writeSeek; prevSampPos++) {
				prevSampPos->filter(0x80, 0x8F);
				prevSampPos->filter(0xE0); }
			
			int dacPos = prevSampPos->getInt();
			for(; prevSampPos < lastWrite; prevSampPos++) {
				if(prevSampPos->filter(0x80, 0x8F)) dacPos++; }
			writeSeek->setInt(dacPos);
		}

		// reset state
		lastWrite = NULL;
		writeSeek = NULL;
		lastSeek = NULL;
		
	,
		switch(cmd) {
		case 0x80 ... 0x8F:
			lastWrite = &event;
			writeSeek = lastSeek;
			break;
		case 0xE0:
			lastSeek = &event;
			break;
		}
	);
}

void vgmEvents_print(
	VgmEvents& events, const char* fName)
{
	FILE* fp = fopen(fName, "w");
	
	for(auto& event : events) {
		fprintf(fp, "EVENT: %d, ", event.sample);
	
	
		if(event) {
			switch(event[0]) {
			case 0x50:
				fprintf(fp, "PSG: %X\n", event[1]);
			case 0x52:
				fprintf(fp, "YMP0: %X, %X\n", event[1], event[2]);
				break;
			case 0x53:
				fprintf(fp, "YMP1: %X, %X\n", event[1], event[2]);
				break;
			case 0xE0:
				fprintf(fp, "SEEK: %X\n", event.getInt());
				break;
			default:	
				fprintf(fp, "%X\n", event[0]);
				break;
			}
		} else {
			fprintf(fp, "null\n");
		}
	}
	
	fclose(fp);
}

void vgmEvents_dedup(VgmEvents& events)
{
	vgmEvents_print(events, "pre-filter.txt");
	filter_dac(events);
	vgmEvents_print(events, "post-filter.txt");
}
