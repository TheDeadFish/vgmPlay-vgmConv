#ifndef _SNDSTATE_H_
#define _SNDSTATE_H_
#include <NextGet.hpp>
#include "2612Unes.hpp"

enum{
	End_InitBlock = 0,
	Pre_InitBlock,
	PreLoop_InitBlock,
	PostLoop_InitBlock,
	};

class sndState
{
public:
	sndState() {}
	
	void Init(char *sampBaseIn, int sampSize,
		int ym2612Unes_mode);

	// ym2612 functions
	ym2612Unes Unes;
	
	// dac functions
	void DWrite(unsigned char data);
	void DacWrite();
	void DacSeek(unsigned char *data);
	void DacProc(void);
	// dac data
	char *sampBase;
	NextGet dacPos;
	char *PreLoop_Seek;
	short PreLoop_Dac;
	bool PostLoop_WriteBad;
	char *PostLoop_Seek;
	short PostLoop_Dac;
	
	// initblock state
	bool loopFound;
	int InitState;
	char *InitBlock_Start;
	char *InitBlock_End;
	
	// initblock functions
	void InitBlock_Begins(char* eventPos);
	void InitBlock_Ends(char* eventPos);
	
	
	void QLoopFound(void);
	void QWrite(int port, char* eventPos);
	void QDacWrite(char* eventPos);
	void QDacSeek(char* eventPos);
};

#endif
