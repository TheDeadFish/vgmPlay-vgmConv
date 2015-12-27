#ifndef _CODEC_H_
#define _CODEC_H_

#include <stdio.h>
#include <MemWrite.hpp>
#include "eventCodec.hpp"
#include "dacCodec.hpp"

class codec
{
public:
	codec(){}
	enum { Wnone, Wevent, Wdac };
	int LastWrite;
	int curSamp;
	eventCodec * EC;
	dacCodec * DC;

	void Init(MemWrite *outData){
		curSamp = 0;
		EC->Init(&curSamp, outData);
		DC->Init(&curSamp, outData);
	}
	
	void eventWrite(int targetSamp, int event, unsigned char *data){
		if(LastWrite == Wdac)
			DC->Flush(targetSamp);
		LastWrite = Wevent;
		EC->Write(targetSamp, event, data);
	}
	
	void dacWrite(int targetSamp){
		if(LastWrite == Wevent)
			EC->Flush(targetSamp);
		LastWrite = Wdac;
		DC->Write(targetSamp);
	}
	
	void Flush(int targetSamp){
		switch(LastWrite){
			case Wnone:
				break;
			case Wevent:
				EC->Flush(targetSamp);
				break;
			case Wdac:
				DC->Flush(targetSamp);
				break;
		}
		LastWrite = Wnone;
	}
	
	void eventFlush(int targetSamp){
		LastWrite = Wnone;
		EC->Flush(targetSamp);
	}
	
	void dacFlush(int targetSamp){
		LastWrite = Wnone;
		DC->Flush(targetSamp);
	}
	
	void *dacGet(int *size){
		return(DC->Get(size));
	}

};

#endif 
