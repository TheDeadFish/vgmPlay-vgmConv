#include "stdshit.h"
#include "vgx/vgmEvent.hpp"
#include <windows.h>
#include "ymRegs.h"

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
		byte reg = ymRegs[addr] & 7;
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
		byte reg = ymRegs[addr];
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
	filter_ym2612_init(events);
	filter_ym2612_dup(events, false);
	
	vgmEvents_print(events, "post-filter.txt");
}
