#ifndef _vgmDac_H_
#define _vgmDac_H_

#include "codec.hpp"
#include "vgmDelay.hpp"

class vgmDac : public dacCodec
{
public:
	vgmDac() {}
	void Init(int *curSampIn, MemWrite *outDataIn);
	void Write(int targetSamp);
	void Flush(int targetSamp);
	void *Get(int *size);

private:
	bool writePend;
	int * curSamp;
	MemWrite * outData;
};

#endif
