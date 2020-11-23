#include "SndState.hpp"

void sndState::Init(char *sampBaseIn, int sampSize,
	int ym2612Unes_mode)
{
	Unes.QInit(ym2612Unes_mode);
	sampBase = sampBaseIn;
	dacPos.maxPos = sampBaseIn + sampSize;
	
	PreLoop_Seek = (char*)0;
	PreLoop_Dac = -1;
	PostLoop_WriteBad = false;
	PostLoop_Seek = (char*)0;
	PostLoop_Dac = -1;
	
	// initblock state
	loopFound = false;
	InitState = Pre_InitBlock;
	InitBlock_Start = NULL;
	InitBlock_End = NULL;
}

void sndState::DWrite(unsigned char data)
{
	switch(InitState){
	case PreLoop_InitBlock:
		PreLoop_Dac = data;
		break;
	case PostLoop_InitBlock:
		PostLoop_Dac = data;
		break;
	default:
		break;
	}
}

void sndState::DacWrite()
{
	if(dacPos.curPos == (char*)0){
		if(InitState == PostLoop_InitBlock);
			PostLoop_WriteBad = true;
	}
	char data = dacPos.Get<char>();
	switch(InitState){
	case PreLoop_InitBlock:
		PreLoop_Seek = dacPos.curPos;
		PreLoop_Dac = data;
		break;
	case PostLoop_InitBlock:
		if(PostLoop_Seek == (char*)0){
			PostLoop_WriteBad = true;
		}else{
			PostLoop_WriteBad = false;
			PostLoop_Seek = dacPos.curPos;
			PostLoop_Dac = data;
		}
		break;
	default:
		break;
	}
}

void sndState::DacSeek(unsigned char *data)
{
	if(sampBase == (char*)0)
		return;
	dacPos.curPos = sampBase + *(unsigned int*)data;	
	switch(InitState){
		case PreLoop_InitBlock:
			PreLoop_Seek = dacPos.curPos;
			break;
		case PostLoop_InitBlock:
			PostLoop_Seek = dacPos.curPos;
			break;
		default:
			break;
	}
}

void sndState::DacProc(void)
{
	if(PostLoop_WriteBad){
		PostLoop_Dac = -1;
		Unes.RegTable[0x2A] = -1;
		PostLoop_Seek = (char*)0;
		PostLoop_Dac = -1;
	}else{
		if(PostLoop_Seek != (char*)0)
			PreLoop_Seek = (char*)0;
		if(PostLoop_Dac != -1)
			PreLoop_Dac = -1;
	}
}

void sndState::InitBlock_Begins(char* eventPos)
{
	if(InitState == Pre_InitBlock) {
		InitBlock_Start = eventPos;
		if(loopFound == true){
			InitState = PostLoop_InitBlock;
		}else{
			InitState = PreLoop_InitBlock;
		}	
	}
}

void sndState::InitBlock_Ends(char* eventPos)
{
	if((InitState != End_InitBlock)
	&&(InitState != Pre_InitBlock))
	{
		DacProc();
		InitBlock_End = eventPos;
		InitState = End_InitBlock;	
	}
}

void sndState::QLoopFound(void)
{
	if(InitState == PreLoop_InitBlock)
		InitState = PostLoop_InitBlock;
	loopFound = true;
	Unes.QLoopFound();
}

void sndState::QWrite(int port, char* eventPos)
{
	if(InitState != End_InitBlock){
		InitBlock_Begins(eventPos);
		if(!port && (eventPos[1] == 0x2A)){
			DWrite((unsigned char)eventPos[2]);
		}else{
			Unes.QWrite(port, (unsigned char*)eventPos+1);
		}
	}else{
		Unes.QWrite(port, (unsigned char*)eventPos+1);
	}
}

void sndState::QDacWrite(char* eventPos)
{
	if(InitState != End_InitBlock){
		InitBlock_Begins(eventPos);
		if(*eventPos != (char)0x80) 
			InitBlock_Ends(eventPos+1);
	}
}
