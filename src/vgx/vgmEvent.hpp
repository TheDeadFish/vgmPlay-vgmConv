#pragma once
#include "Vgm.hpp"

struct VgmEvent
{
	int sample;
	byte* data;
	
	
	operator byte*() { return data; }
	
	
	int getCmd() { return data ? data[0] : -1; }
	
	
	bool filter(int type) { if(getCmd() == type) {
		data = NULL; return true; } return false; }
	bool filter(int first, int last) {
		if(inRng(getCmd(), first, last)) {
		data = NULL; return true; } return false; }
	
	int getInt(size_t i = 1) { return *(int*)(data+i); }
	void setInt(int x, size_t i = 1) { *(int*)(data+i) = x; }
	
};

struct VgmEvents
{
	VgmEvent* events;
	VgmEvent* loopPos;	
	size_t nEvents;
	int initSample;
	
	VgmEvents() { ZINIT; }
	~VgmEvents() { free(events); }
	
	bool loop(VgmEvent& event) {
		return &event == loopPos; }
	
	VgmEvent* begin() { return events; }
	VgmEvent* end() { return events+nEvents; }
	
	bool init(VgmFile_& vgmInfo);
};


#define VGMEVENT_FILTER(events, atLoop, atFrame, main) { \
	int prevSamp = -1; VgmEvent* prevSampPos = NULL; \
	for(auto& event : events) { int cmd = event.getCmd(); \
		if((events.loop(event) && (({atLoop}), 1)) \
		||(prevSamp != event.sample)||(cmd == 0x66)) { \
			atFrame; prevSampPos = &event; } \
		main; prevSamp = event.sample; } }
