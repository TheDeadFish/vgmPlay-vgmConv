#include "stdshit.h"
#include "vgx/vgmEvent.hpp"
#include <windows.h>

enum {
	YMGRP_CHN0 = 0x80,
	YMGRP_CHN2 = 0x82,
	YMGRP_CHN3 = 0x84,
	YMGRP_CHNX = 0x83,
	YMGRP_ALWAYS = 0x87,
	YMGRP_NEVER = 0
};

int ymRegs(int addr)
{
	byte bAddr = addr;

	if(inRng(bAddr, 0x30, 0xB7)) {
		if((bAddr & 3) == 3) 
			return YMGRP_NEVER;
		if((bAddr & 0xF8) == 0xA8)
			return YMGRP_CHN2;
		
		if(addr > 255) {
			return YMGRP_CHN3 | (bAddr & 3);
		} else {
			return YMGRP_CHN0 | (bAddr & 3);
		}
	}
	
	switch(addr) {
	case 0x22: return YMGRP_CHNX;
	case 0x27: return YMGRP_CHNX;
	case 0x28: return YMGRP_ALWAYS;
	case 0x2A: return YMGRP_ALWAYS;
	case 0x2B: return YMGRP_ALWAYS;
	default:
		return YMGRP_NEVER;
	}
}


struct FilterDac
{
	VgmEvent* firstWrite;
	VgmEvent* lastWrite8n;
	VgmEvent* lastWrite2A;
	VgmEvent* lastSeek8n;
	VgmEvent* lastSeek;
	int dacPos, dacPos8n;

	FilterDac() { reset(); }
	void reset() { ZINIT; }
	
	void write(VgmEvent& event) {
		if(!firstWrite) firstWrite = &event;
	}	
	
	void seek(VgmEvent& event) {
		lastSeek = &event;
		dacPos = event.getInt();
		this->write(event);
	}
	
	void write8n(VgmEvent& event) {
		lastWrite8n = &event;
		lastSeek8n = lastSeek;
		dacPos8n = dacPos++;
		this->write(event);
	}
	
	void write2A(VgmEvent& event) {
		lastWrite2A = &event;
		this->write(event);
	}
	
	void flush(VgmEvent& end)
	{
		if(!firstWrite) return;
		
		if(lastWrite2A < lastWrite8n)
			lastWrite2A = NULL;
		if(lastWrite8n < lastWrite2A) {
			lastWrite8n = NULL; dacPos8n++; }
		if(lastSeek8n) {
			lastSeek8n->setInt(dacPos8n);
			if((lastSeek8n == lastSeek)
			&&(lastWrite8n == NULL))
				lastSeek8n = NULL; }
		
		for(auto* x = firstWrite; x < &end; x++) {
			switch(x->getCmd()) {
			case 0x80 ... 0x8F:
				if(lastSeek
				&&(x != lastWrite8n))
					x->kill();
				break;
			case 0xE0:
				if((x != lastSeek8n)
				&&(x != lastSeek))
					x->kill();
				break;
			case 0x52:
				if((x->data[1] == 0x2A)
				&&(x != lastWrite2A))
					x->kill();
				break;
			}
		}
	}
};


void filter_dac(VgmEvents& events)
{
	FilterDac filterDac;
	byte dacEnable = 0;
	
	VGMEVENT_FILTER(events,
		dacEnable = -1;
	,
		if(dacEnable & 0x80) {
			filterDac.flush(event);
			filterDac.reset();
		}
	,
		switch(cmd) {
		case 0x80 ... 0x8F:
			filterDac.write8n(event);
			break;
		case 0xE0:
			filterDac.seek(event);
			break;
		case 0x52:
			if(event[1] == 0x2B)
				dacEnable |= event[2];
			if(event[1] == 0x2A) {
				filterDac.write2A(event); }
			break;
		}
	);
}

struct YmPtrList {
	VgmEvent** data;
	YmPtrList() { data = (VgmEvent**)calloc(512, sizeof(VgmEvent*)); }
	~YmPtrList() { free(data); }
	
	void kill(int addr) {
		if(data[addr]) data[addr]->data = NULL; }
	void set(int addr, VgmEvent& event) {
		data[addr] = &event; }
	void reset(int addr) { data[addr] = NULL; }
	void noteOn(byte data);
};

void YmPtrList::noteOn(byte data)
{
	data &= 7;
	for(int addr = 0; addr < 512; addr++) {
		byte reg = ymRegs(addr) & 7;
		if((reg == 3)||(reg == data))
			reset(addr);
	}
}


struct KeyMask {
	byte mask[8];
	
	
	KeyMask() { ZINIT; mask[7] = 0xF0; }
	
	
	byte& operator[](size_t i) { return mask[i]; }
	
	byte write(int addr, int data);
	
	void noteOn(byte data) {
		mask[data & 7] |= data & 0xF0;
		mask[3] |= data & 0xF0; }
};

byte KeyMask::write(int addr, int data) {
	if(addr != 0x28) return false;
	noteOn(data); return data & 0xF0; }

struct DacMask {
	byte mask = 0;
	
	byte write(int addr, int data) {
		if(addr != 0x2B) return false;
		mask |= data; return data; }
};


#define YM2612_EVENT(...) if(inRng(cmd, 0x52, 0x53)) { \
	int addr = event[1]; int data = event[2]; \
	if(cmd == 0x53) addr += 256; __VA_ARGS__; }
	
void filter_freq_fix(VgmEvents& events)
{
	struct SwapInfo { u16 addr; byte index; };
	static const SwapInfo swapInfo[18] = {
		{0x0A4, 0}, {0x1A4, 6}, {0x0A5, 6}, {0x1A5, 7}, {0x0A6, 6}, {0x1A6, 8},
		{0x0A0, 7}, {0x1A0, 9}, {0x0A1, 9}, {0x1A1, 10}, {0x0A2, 10}, {0x1A2, 11},
		{0x0AC, 12}, {0x0AD, 15}, {0x0AE, 15}, {0x0A8, 16}, {0x0A9, 16}, {0x0AA, 17}
	};
	
	byte** pLst[18];
	int matchLen = 0;
	
	VGMEVENT_FILTER(events,
		 matchLen = 0;
	,,
		if(events.initEnd(event)) break;
		YM2612_EVENT(
			if(swapInfo[matchLen].addr != u16(addr)) matchLen = 0;
			else { pLst[matchLen] = &event.data; matchLen++;
				if(matchLen == 18) {
					for(int i = 0; i < 18; i++)
						swap(*pLst[i], *pLst[swapInfo[i].index]);
					matchLen = 0;
				}
			}
		);
	);
}

void filter_ym2612_init(VgmEvents& events)
{
	// get last keyOn mask
	KeyMask lastKeyMask;
	DacMask lastDacMask;
	VGMEVENT_FILTER(events,,,
	YM2612_EVENT(
		lastKeyMask.write(addr, data);
		lastDacMask.write(addr, data);
	));

	// kill redundant writes
	YmPtrList pLst;
	KeyMask keyMask;
	DacMask dacMask;
	VGMEVENT_FILTER(events,
		keyMask = lastKeyMask;
		dacMask = lastDacMask;
		
	,,
	YM2612_EVENT(
		byte reg = ymRegs(addr);
		if(!reg || !lastKeyMask[reg&7]) {
			if((addr == 0x1B6)&&(lastDacMask.mask))
				goto SKIP_KILL;
			event.data = NULL;
		} else {
		SKIP_KILL:
			if(keyMask.write(addr, data))
				pLst.noteOn(data);
			if(dacMask.write(addr, data))
				pLst.reset(0x1B6);
			
			pLst.kill(addr);
			if((addr & 0xF8) == 0xA8) goto SKIP_SET;
			if(((addr & 0xF0) == 0x90)&&(data)) goto SKIP_SET;
			if(keyMask[reg&7]) goto SKIP_SET;
			if((addr == 0x1B6)&&(dacMask.mask)) goto SKIP_SET;
			pLst.set(addr, event);
		SKIP_SET:;
		}
	));
}

struct Ym2612State {
	short regs[512];
	
	Ym2612State();
	
	bool write(int addr, int data, bool hwMode);
	
	bool write(int addr, int data);
	bool writeFreq(int addr, int data);
	
	void loop(Ym2612State& that);
	
};

Ym2612State::Ym2612State()
{
	ZINIT;
	regs[0xB0] = 0xC0; regs[0x1B0] = 0xC0;
	regs[0xB1] = 0xC0; regs[0x1B1] = 0xC0;
	regs[0xB2] = 0xC0; regs[0x1B2] = 0xC0;
}

void Ym2612State::loop(Ym2612State& that)
{
	for(int i = 0; i < 512; i++) {
		if(regs[i] != that.regs[i])
			regs[i] = -1; }
}

bool Ym2612State::writeFreq(int addr, int data)
{
	int latchAddr = 0x10 | (addr & 4);
	if(addr & 8) { return write(latchAddr, data); }
	else { return write(addr, data)
		|| write(addr|4, regs[latchAddr]); }
}

bool Ym2612State::write(int addr, int data)
{
	if(addr == 0x28) { addr = data & 7; data &= 0xF0; }
	int oldVal = regs[addr]; regs[addr] = data;
	return (oldVal != data)||(data < 0);
}

bool Ym2612State::write(int addr, int data, bool safeMode)
{
	if((addr & 0xF0) == 0xA0) {
		return writeFreq(addr, data) || safeMode;
	} else {
		return write(addr, data);
	}
}

void filter_ym2612_dup(VgmEvents& events, bool safeMode)
{
	Ym2612State lastState;
	VGMEVENT_FILTER(events,,,
	YM2612_EVENT(
		lastState.write(addr, data, safeMode);
	));
	
	Ym2612State state;
	VGMEVENT_FILTER(events,
		state.loop(lastState);
	,,
	YM2612_EVENT(
		if(!state.write(addr, data, safeMode))
			event.data = NULL;
	));
}


void filter_ym2612_freq(VgmEvents& events, bool safeMode)
{
	if(safeMode) return;

	VgmEvent* regs[2] = {};
	VGMEVENT_FILTER(events,,,
	YM2612_EVENT(
		if((addr & 0xF0) == 0xA0) {
			int addrH = (addr & 8) / 8;
			if(addr & 4) {
				if(regs[addrH]) { regs[addrH]->data = NULL;	}
				regs[addrH] = &event;
			} else { regs[addrH] = NULL; }
		}
	));
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
				break;
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
	filter_freq_fix(events);
	filter_dac(events);
	filter_ym2612_init(events);
	filter_ym2612_dup(events, false);
	filter_ym2612_freq(events, false);
	
	vgmEvents_print(events, "post-filter.txt");
}
