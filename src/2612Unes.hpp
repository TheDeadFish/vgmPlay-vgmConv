#ifndef _2612UNES_H_
#define _2612UNES_H_
#include "MemWrite.hpp"

class ym2612Unes
{
public:
	void QInit(int mode);
	void QWrite(int port, unsigned char *data);
	void QLoopFound(void);
	void Init(int mode);
	bool Write(int port, unsigned char *data);
	void LoopFound(void);
	void SetMode(int mode) {
		this->mode = mode; }
		
//private:
	void InvalidateFreq();
	bool freqEmu(int addr, int data);
	void freqHdw(int addr, int data);
	void holdList_write(char keep);
	void keepPend(int index);

	// basic state
	int mode;
	short RegTable[512];
	short oldRegs[512];
	
	// frequency state
	MemWrite holdList;
	int holdPend[2];
	short holdState[2];
	short holdState2[2];
};

#endif
