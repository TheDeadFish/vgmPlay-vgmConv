#ifndef _vgcEvent_H_
#define _vgcEvent_H_

#include "codec.hpp"
#include "vgcDelay.hpp"

class vgcEvent : public eventCodec
{
public:
	vgcEvent() {}
	void Init(int *curSampIn, MemWrite *outDataIn);
	void Write(int targetSamp, int event, unsigned char *data);
	void Flush(int targetSamp);
	void State(int state);
	
private:
	int * curSamp;
	MemWrite * outData;
	int eventPend;
	unsigned char * dataPend;
};

#endif
