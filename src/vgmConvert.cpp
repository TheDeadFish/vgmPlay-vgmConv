#include "vgmConv.hpp"
#include "codec/codecs.hpp"
#include "SndState.hpp"
#include "dacStream.hpp"
#include "sampScale.hpp"
#include <NextGet.hpp>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <new>
using namespace std;

#define MinInitBlock 16

#define InitBlock_Ends(back){\
		SndS.DacProc(); \
		InitBlock_End = vgmPos.curPos - back; \
		InitState = End_InitBlock;}


#define InitBlock_Begins(back){\
		InitBlock_Start = vgmPos.curPos - back; \
		if(loopFound == true){ \
			InitState = PostLoop_InitBlock; \
		}else{ \
			InitState = PreLoop_InitBlock;  \
		}}

int VgmConv::vgmConvert(vgmFile& vgmInfo)
{
	// Main variables
	unsigned char event;
	char *loopIndexP;
	
	// pass1 Variables
	bool loopFound = false;
	int InitState = Pre_InitBlock;
	char *InitBlock_Start = NULL;
	char *InitBlock_End = NULL;
	
	// Setup loopIndexP
	if(vgmInfo.loopIndex != 0xffffffff){
		loopIndexP = vgmInfo.mainData + vgmInfo.loopIndex;
	}else
	loopIndexP = NULL;

	// Sound state setup
	sndState SndS;
	SndS.Init(vgmInfo.sampData, vgmInfo.sampSize, dupRemove);

	// pass 1 exception block
	{
		jmp_buf jmpBuf;
		NextGet vgmPos(
		vgmInfo.mainData, vgmInfo.mainSize,	jmpBuf, 1);
		SndS.Unes.holdList.SetExcp(&jmpBuf, 2);
		if(int result = setjmp( jmpBuf )) return
			(result == 2) ? VGX_MEM_ERR : VGX_FILE_BAD;
		
		// begin pass1 Loop
		for(;;){
			// Check loop
			if((loopIndexP != NULL)&&(loopFound == false)){
				if(vgmPos.curPos > loopIndexP)
				return VGX_FILE_BAD;
				if(vgmPos.curPos == loopIndexP){
					if(InitState == PreLoop_InitBlock)
					InitState = PostLoop_InitBlock;
					loopFound = true;
					SndS.Unes.QLoopFound();
				}
			}
			event = vgmPos.Get<char>();
			switch(event){
			case 0x50: // PSG Write
				if(InitState == Pre_InitBlock)
				InitBlock_Begins(1);
				vgmPos.Add(1);
				continue;
			case 0x52: // YM2612 Port0
				vgmPos.Range(2);
				if(InitState != End_InitBlock){
					if(InitState == Pre_InitBlock)
					InitBlock_Begins(1);
					if(vgmPos.curPos[0] == 0x2A){
						SndS.DWrite(InitState, (unsigned char)vgmPos.curPos[1]);
					}else{
						SndS.Unes.QWrite(0, (unsigned char*)vgmPos.curPos);
					}
				}else{
					SndS.Unes.QWrite(0, (unsigned char*)vgmPos.curPos);
				}
				vgmPos.Add(2);
				continue;
			case 0x53: // YM2612 Port0
				if(InitState == Pre_InitBlock)
				InitBlock_Begins(1);
				vgmPos.Range(2);
				SndS.Unes.QWrite(1, (unsigned char*)vgmPos.curPos);
				vgmPos.Add(2);
				continue;
			case 0x80 ... 0x8f: // Dac Write
				if(InitState != End_InitBlock){
					if(InitState == Pre_InitBlock)
					InitBlock_Begins(0);
					if(event > 0x80)
					InitBlock_Ends(0);
				}	
				continue;
			case 0x61:{
					short delay = vgmPos.Get<short>();
					if((InitState != End_InitBlock)&&(InitState != Pre_InitBlock)&&(delay != 0))
					InitBlock_Ends(3);
					continue;}
			case 0x70 ... 0x7f:
			case 0x62 ... 0x63:
				if((InitState != End_InitBlock)&&(InitState != Pre_InitBlock))
				InitBlock_Ends(1);
				continue;
			case 0xe0:
				if(InitState != End_InitBlock){
					vgmPos.Range(4);
					SndS.DacSeek(InitState, (unsigned char*)vgmPos.curPos);
				}
				vgmPos.Add(4);
				continue;
			case 0x66:
				break;
			default:{
					int tmp = vgmInfo.ELen(event);
					if(tmp == -1)
					return VGX_FILE_BAD;
					vgmPos.Add(tmp);
					continue;}
			}
			break;
		}}
	
	
	// Check initBlock bounds and initBlock option
	bool Write_Dacs;
	int init_dupRemove = dupRemove;
	if((InitBlock_Start != NULL)&&(InitBlock_End != NULL)&&
			(InitBlock_End - InitBlock_Start >= MinInitBlock)&&
			(options & writeInitB)){
		InitState = Pre_InitBlock;
		Write_Dacs = false;
		if(init_dupRemove == 0)
			init_dupRemove = 1;
	}else{
		InitState = End_InitBlock;
		Write_Dacs = true;
	}	
	
	// pass2 variables
	SampScale curSamp(sscale);
	int loopSamp = 0;
	NxtGet vgmPos;
	vgmPos = NxtGet(vgmInfo.mainData);
	SndS.Unes.Init(init_dupRemove);
	MemWrite outData;
	
	// V1.60 dac stream filter
	dacStream Codec;
	Codec.vgmInfo = &vgmInfo;
	
	// Select codec
	unique_ptr<eventCodec> EC;
	unique_ptr<dacCodec> DC;
	if(options & vgcFormat)
	{
		EC.reset( new (nothrow) vgcEvent() );
		DC.reset( new (nothrow) vgcDac() );
		if(DC.get() == 0)
			return VGX_MEM_ERR;
		if(options & avSamp)
			((vgcDac*)DC.get())->mode = 1;
	}
	else
	{
		EC.reset( new (nothrow) vgmEvent() );
		DC.reset( new (nothrow) vgmDac() );
		if(DC.get() == 0)
			return VGX_MEM_ERR;
	}
	if(EC.get() == 0)
		return VGX_MEM_ERR;
	Codec.EC = EC.get();
	Codec.DC = DC.get();
	Codec.Init(&outData);
	
	// pass 2 exception block
	{
		jmp_buf jmpBuf;
		outData.SetExcp(&jmpBuf, 1);
		if(setjmp( jmpBuf ) != 0)
		return VGX_MEM_ERR;
		
		// Begin pass2 Loop
		for(;;){
			if(InitState != End_InitBlock){
				if(vgmPos.curPos == InitBlock_Start){
					unsigned char data[5];
					if(SndS.PreLoop_Dac != -1){
						data[0] = 0x2A;
						data[1] = SndS.PreLoop_Dac;
						Codec.eventWrite(curSamp, EventYMP0, &data[0]);
					}
					if(SndS.PreLoop_Seek != (char*)0){
						*(long*)data = SndS.PreLoop_Seek - vgmInfo.sampData;
						Codec.eventWrite(curSamp, EventSEEK, &data[0]);
					}
					InitState = PreLoop_InitBlock;
				}
				if(vgmPos.curPos == InitBlock_End){
					InitState = End_InitBlock;
					Write_Dacs = true;
					SndS.Unes.SetMode(dupRemove);
				}
			}
			if((loopFound == true)&&(vgmPos.curPos == loopIndexP)){
				loopSamp = curSamp;
				Codec.Flush(curSamp);
				vgmInfo.loopIndex = outData.curIndex();
				loopFound = false;
				SndS.Unes.LoopFound();
				
				// Deal with initBlock
				if(InitState == PreLoop_InitBlock){
					if(SndS.PostLoop_WriteBad == false){
						unsigned char data[5];
						if(SndS.PostLoop_Dac != -1){
							data[0] = 0x2A;
							data[1] = SndS.PostLoop_Dac;
							Codec.eventWrite(curSamp, EventYMP0, &data[0]);
						}
						if(SndS.PostLoop_Seek != (char*)0){
							*(long*)data = SndS.PostLoop_Seek - vgmInfo.sampData;
							Codec.eventWrite(curSamp, EventSEEK, &data[0]);
						}
					}else
					Write_Dacs = true;
					InitState = PostLoop_InitBlock;
				}
			}
			
			event = vgmPos.Get<char>();
			switch(event){
			case 0x50: // PSG Write
				Codec.eventWrite(curSamp, EventPSGP, (unsigned char*)vgmPos.curPos);
				vgmPos.Add(1);
				continue;
			case 0x52:{ // YM2612 Port0
					if(vgmPos.curPos[0] == 0x2A){
						if(Write_Dacs)
							Codec.eventWrite(curSamp, EventYMP0, (unsigned char*)vgmPos.curPos);
					}else{ 
						if(SndS.Unes.Write(0, (unsigned char*)vgmPos.curPos))
							Codec.eventWrite(curSamp, EventYMP0, (unsigned char*)vgmPos.curPos);
					}
					vgmPos.Add(2);
					continue;}
			case 0x53: // YM2612 Port1
				if(SndS.Unes.Write(1, (unsigned char*)vgmPos.curPos))
					Codec.eventWrite(curSamp, EventYMP1, (unsigned char*)vgmPos.curPos);
				vgmPos.Add(2);
				continue;
			case 0xe0: // Dac seek
				if(Write_Dacs)
				Codec.eventWrite(curSamp, EventSEEK, (unsigned char*)vgmPos.curPos);
				vgmPos.Add(4);
				continue;
			case 0x4f: // Psg Stereo
				Codec.eventWrite(curSamp, EventPSGS, (unsigned char*)vgmPos.curPos);
				vgmPos.Add(1);
				continue;
			case 0x66: // Vgm end
				Codec.eventWrite(curSamp, Event_END, (unsigned char*)vgmPos.curPos);
				Codec.Flush(curSamp);
				break;
			case 0x80 ... 0x8f: // Dac Write
				if(InitState != 0){	
					if(Write_Dacs)
					Codec.dacWrite(curSamp);
					Codec.Flush(curSamp);
				}else
				Codec.dacWrite(curSamp);
				curSamp += event & 0x0f;
				continue;
			case 0x61: // 0x61 nn nn delay	
				curSamp += vgmPos.Get<unsigned short>();
				continue;
			case 0x62: // 735 delay
				Codec.Flush(curSamp);
				curSamp += 735;
				continue;
			case 0x63: // 882 delay
				Codec.Flush(curSamp);
				curSamp += 882;
				continue;
			case 0x70 ... 0x7f: // 0x7n 
				curSamp += (event & 0x0f) + 1;
				continue;
				// V1.60 dac stream events
			case 0x90 ... 0x95:
				Codec.dsWrite(curSamp, (unsigned char*)vgmPos.curPos);
				vgmPos.Add( vgmInfo.ELen(event) );
				continue;
			default:{	
					int tmp = vgmInfo.ELen(event);
					if(tmp == -1)
					return VGX_FILE_BAD;
					vgmPos.Add(tmp);
					continue;
				}
			}
			break;
		}
	}
	
	// output to file
	if(options & vgcFormat)
	{
		vgcFile vgcInfo;
		vgcInfo.mainData = outData.getBase();
		vgcInfo.mainSize = outData.curIndex();
		vgcInfo.loopIndex = vgmInfo.loopIndex;
		vgcInfo.sampData = vgmInfo.sampData;
		vgcInfo.sampSize = vgmInfo.sampSize;
		vgcInfo.gd3Data = vgmInfo.gd3Data;
		vgcInfo.gd3Size = vgmInfo.gd3Size;
		vgmInfo.samples = curSamp;
		vgmInfo.loopSamp = curSamp - loopSamp;
		if(dest == NULL)
			fileData = vgcInfo.Save(fileSize);
		else
			vgcInfo.Save(dest);
		return vgcInfo.status;
	}
	else
	{
		vgmInfo.mainData = outData.getBase();
		vgmInfo.mainSize = outData.curIndex();
		if(dest == NULL)
			fileData = vgmInfo.Save(fileSize);
		else
			vgmInfo.Save(dest, options & compressO);
		return vgmInfo.status;
	}
}
