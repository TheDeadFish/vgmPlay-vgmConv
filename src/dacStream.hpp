#ifndef INC_DACSTREAM_H_
#define INC_DACSTREAM_H_
#include "codec/codec.hpp"
#include "vgx/Vgx.hpp"

class dacStream : public codec
{
public:
	vgmFile* vgmInfo;
	// playback vars
	float sample_delay;
	float remain_delay;
	int remain_len;
	int current_samp;
	
	dacStream()
	{	current_samp = -1;	}
	
	// vgm dac events are undefined when dacStream is used
	void dacWrite(int targetSamp)
	{ 	codec::dacWrite(targetSamp); }
	
	void eventWrite(int targetSamp, int event, unsigned char *data);
	void Flush(int targetSamp);
	void dsOutput(int targetSamp);
	bool dsWrite(int targetSamp, unsigned char *data);
	bool start(int targetSamp, int blockId,
		int seekPos, int mode, int length);
		
	// initblock
	void initBlock(int curSamp, int dac, char* seek);
	
	
	
	
	
	
};

#endif
