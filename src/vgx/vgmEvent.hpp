#pragma once
#include "Vgm.hpp"

struct VgmEvent
{
	int sample;
	byte* data;
	
	
	operator byte*() { return data; }
	
	
	//byte operator[]
	
};

struct VgmEvents
{
	VgmEvent* events;
	VgmEvent* loopPos;	
	size_t nEvents;
	int initSample;
	
	VgmEvents() { ZINIT; }
	~VgmEvents() { free(events); }
	
	
	VgmEvent* begin() { return events; }
	VgmEvent* end() { return events+nEvents; }
	
	bool init(VgmFile_& vgmInfo);
};
