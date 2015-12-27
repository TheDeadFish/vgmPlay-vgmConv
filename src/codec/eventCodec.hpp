// Should only be included by codec.hpp

// events
enum{
	EventNULL,
	EventPSGP,
	EventYMP0,
	EventYMP1,
	EventSEEK,
	EventPSGS,
	Event_END
};

enum{
	Normal_Block,
	ym2612Init_Block,
	psgInit_Block,
	ym2612Note_Block,
	psgNote_Block,
	dacSeek_Block
};

// event states
#define StateInitBlock 0
#define StateNormBlock 1

class eventCodec
{
public:
	eventCodec() {}
	virtual void Init(int *curSampIn, MemWrite *outDataIn)=0;
	virtual void Write(int targetSamp, int event, unsigned char *data)=0;
	virtual void Flush(int targetSamp)=0;
	virtual void State(int state)=0;
};
