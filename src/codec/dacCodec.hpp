// Should only be included by codec.hpp

class dacCodec
{
public:
	dacCodec() {}
	virtual void Init(int *curSampIn, MemWrite *outDataIn)=0;
	virtual void Write(int targetSamp)=0;
	virtual void Flush(int targetSamp)=0;
	virtual void *Get(int *size)=0;
};
