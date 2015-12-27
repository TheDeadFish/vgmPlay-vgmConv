#include "stdshit.h"
#include "2612Unes.hpp"
#define OPN_SLOT(N) ((N>>2)&3)
static inline int neg2ToNeg1(int in) {
	return (in == -2) ? -1 : in; }
	
DEF_RETPAIR(isValidWrite_t, int, addr, int, data);
static 
isValidWrite_t isValidWrite(int port, byte* data)
{
	isValidWrite_t result =
		{data[0], data[1]};
	switch(result.addr)
	{
	case 0x22:	
	case 0x27 ... 0x28:
	case 0x2A ... 0x2B:
		if(port != 0) break;
		return result;
	case 0xA8 ... 0xAF:
		if(port != 0) break;
	case 0x30 ... 0xA7:
	case 0xB0 ... 0xB7:
		if((result.addr&3) == 3) break;
		if(port) result.addr += 0x100;
		return result;
	}
	return isValidWrite_t(0);
}

void ym2612Unes::QInit(int mode) {
	holdList.CurPos = 0;
	this->mode = mode ? mode : 1;
	for(int i = 511; i >= 0; i--)
		RegTable[i] = -1; 
	this->InvalidateFreq(); }
void ym2612Unes::Init(int mode) {
	keepPend(0); keepPend(1);
	memcpy(oldRegs, RegTable, sizeof(oldRegs));
	this->QInit(mode); this->mode = mode; 
	holdList.CurPos = holdList.Base; }
void ym2612Unes::LoopFound(void) {
	for(int i = 511; i >= 0; i--)
		if(RegTable[i] != oldRegs[i])
			RegTable[i] = -1;
	this->InvalidateFreq(); }
void ym2612Unes::QLoopFound(void) {
	keepPend(0); keepPend(1);
	this->InvalidateFreq(); }

void ym2612Unes::QWrite(int port, unsigned char *data)
{
	if(auto wdat = isValidWrite(port, (byte*)data))
	{
		if((wdat.addr & 0xf0) != 0xA0)
			RegTable[wdat.addr] = wdat.data;
		ei(mode == 3)
			freqHdw(wdat.addr, wdat.data); 
	}
}

bool ym2612Unes::Write(int port, unsigned char *data)
{
	if(mode == 0) return true;
	auto wdat = isValidWrite(port, (byte*)data);
	if(wdat.addr == 0) return false;
	if((wdat.addr & 0xf0) != 0xA0) {
		if(RegTable[wdat.addr] == wdat.data) return false;
		RegTable[wdat.addr] = wdat.data; return true; }

	// frequency special
	if(mode == 2) return freqEmu(wdat.addr, wdat.data);
	if(mode == 3) return *holdList.CurPos++ > 0;
	return true;
}

bool ym2612Unes::freqEmu(int addr, int Data)
{
	if(RegTable[addr] == Data) 
		return false;
	RegTable[addr] = Data;
	if((addr & 0xF4) == 0xA4)
		RegTable[addr-4] = -1;
	return true; 
}

void ym2612Unes::freqHdw(int addr, int Data)
{
	int index = (addr & 8) ? 1 : 0;
	if((addr & 4) != 0) 
	{
		if(holdPend[index] >= 0) {
			if(holdList.Base[holdPend[index]] > 0)
				holdState2[index] = holdState[index];
			holdPend[index] = -1; }
		holdPend[index] = holdList.curIndex();
		bool keep = holdState2[index] != Data;
		holdList_write(keep ? 0 : -1);
		holdState[index] = Data;
	} else {
		bool keep = (RegTable[addr] != Data)
			|| (RegTable[addr+4] != holdState[index]);
		RegTable[addr] = Data;
		RegTable[addr+4] = neg2ToNeg1(holdState[index]);
		holdList_write(keep);
		if(keep == true) keepPend(index);
	}
}

void ym2612Unes::holdList_write(char keep)
{
	if((holdList.Base == NULL)
	&&(!holdList.Alloc(65536)))
		longjmp(*holdList.jmpBuf, holdList.excpCode);
	holdList.Write(keep);
}

void ym2612Unes::keepPend(int index)
{
	if(holdPend[index] >= 0)
		holdList.Base[holdPend[index]] |= 1;
}

void ym2612Unes::InvalidateFreq(void)
{
	for(int i = 0; i < 16; i++) {
		RegTable[0x0A0+i] = -1;
		RegTable[0x1A0+i] = -1; }
	for(int i = 0; i < 2; i++) {
		holdPend[i] = -1;
		holdState[i] = -2;
		holdState2[i] = -2;	}	
}
