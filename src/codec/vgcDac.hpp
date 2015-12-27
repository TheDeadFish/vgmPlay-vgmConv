#ifndef _vgcDac_H_
#define _vgcDac_H_

#include "codec.hpp"
#include "vgcDelay.hpp"

#define MAX_SAMP 4096

class vgcDac : public dacCodec
{
public:
	bool mode;
	// original functions
	vgcDac() { mode = 0;}
	void Init(int *curSampIn, MemWrite *outDataIn);
	void Write(int targetSamp);
	void Flush(int targetSamp);
	void *Get(int *size);
	
	// new functions
	int rleSearch(int index, int max, int size);
	void rleWrite(int size);
	void rleWrite(unsigned char c);
	void rleFlush(void);

private:
	// Original variables
	void Core(int targetSamp, bool pendState);
	bool writePend;
	int * curSamp;
	MemWrite * outData;
	
	// New Varibles
	char cmd8n[MAX_SAMP];
	int idx8n;
	unsigned char rle8_2Min;
	int rle8_2Index;
	int rle8_2End;
	unsigned char rle8_1Min;
	int rle8_1Index;
	int rle8_1End;
};

#endif
