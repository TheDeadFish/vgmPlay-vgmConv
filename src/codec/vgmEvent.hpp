#ifndef _vgmEvent_H_
#define _vgmEvent_H_

#include "codec.hpp"
#include "vgmDelay.hpp"

class vgmEvent : public eventCodec
{
public:
	vgmEvent() {}
	void Init(int *curSampIn, MemWrite *outDataIn);
	void Write(int targetSamp, int event, unsigned char *data);
	void Flush(int targetSamp);
	void State(int state);

private:
	int * curSamp;
	MemWrite * outData;
};

#endif
