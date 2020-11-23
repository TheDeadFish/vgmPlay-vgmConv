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
#include "stdshit.h"
using namespace std;

#define MinInitBlock 16

int VgmConv::vgmConvert(vgmFile& vgmInfo)
{
	// Main variables
	unsigned char event;

	// Sound state setup
	sndState SndS;
	SndS.Init(vgmInfo.sampData, vgmInfo.sampSize, dupRemove);

	// pass 1 exception block
	{
		jmp_buf jmpBuf;
		VgmPos vgmPos(vgmInfo);

		SndS.Unes.holdList.SetExcp(&jmpBuf, 2);
		if(int result = setjmp( jmpBuf )) return
			(result == 2) ? VGX_MEM_ERR : VGX_FILE_BAD;
		
		// begin pass1 Loop
		for(;;){
			// Check loop
			if(vgmPos.loop())
				SndS.QLoopFound();
			
			char* eventPos = vgmPos.next();
			event = *eventPos;

			switch(event){
			case 0x50: // PSG Write
				SndS.InitBlock_Begins(eventPos);
				continue;
			case 0x52: // YM2612 Port0
				SndS.QWrite(0, eventPos);
				continue;
			case 0x53: // YM2612 Port0
				SndS.QWrite(1, eventPos);
				continue;
			case 0x80 ... 0x8f: // Dac Write
				SndS.QDacWrite(eventPos);
				continue;
			case 0x61:{
					short delay = *(short*)(eventPos+1);
					if(delay) SndS.InitBlock_Ends(eventPos);
					continue;}
			case 0x70 ... 0x7f:
			case 0x62 ... 0x63:
				SndS.InitBlock_Ends(eventPos);
				continue;
			case 0xe0:
				if(SndS.InitState != End_InitBlock){
					SndS.DacSeek((unsigned char*)eventPos+1);
				}
				continue;
			case 0x66:
				break;
			default:
					continue;
			}
			break;
		}}
		
	printf("%X, %X\n", SndS.InitBlock_Start-vgmInfo.mainData, 
		SndS.InitBlock_End-vgmInfo.mainData);
		
	
	
	// Check initBlock bounds and initBlock option
	bool Write_Dacs;
	int init_dupRemove = dupRemove;
	if((SndS.InitBlock_Start != NULL)&&(SndS.InitBlock_End != NULL)&&
			(SndS.InitBlock_End - SndS.InitBlock_Start >= MinInitBlock)&&
			(options & writeInitB)){
		SndS.InitState = Pre_InitBlock;
		Write_Dacs = false;
		if(init_dupRemove == 0)
			init_dupRemove = 1;
	}else{
		SndS.InitState = End_InitBlock;
		Write_Dacs = true;
	}	
	
	// pass2 variables
	SampScale curSamp(sscale);
	int loopSamp = 0;
	VgmPos vgmPos(vgmInfo);
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
			if(SndS.InitState != End_InitBlock){
				if(vgmPos.curPos == SndS.InitBlock_Start){
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
					SndS.InitState = PreLoop_InitBlock;
				}
				if(vgmPos.curPos == SndS.InitBlock_End){
					SndS.InitState = End_InitBlock;
					Write_Dacs = true;
					SndS.Unes.SetMode(dupRemove);
				}
			}
			
			if(vgmPos.loop()) {
				loopSamp = curSamp;
				Codec.Flush(curSamp);
				vgmInfo.loopIndex = outData.curIndex();
				SndS.loopFound = false;
				SndS.Unes.LoopFound();
				
				// Deal with initBlock
				if(SndS.InitState == PreLoop_InitBlock){
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
					SndS.InitState = PostLoop_InitBlock;
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
				if(SndS.InitState != 0){	
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
		
		vgcInfo.extraData = (char*)romName;
		vgcInfo.extraSize = strsize(romName);

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
