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
			char* eventPos = vgmPos.next();
		
		
			// Check loop
			if(vgmPos.loop())
				SndS.QLoopFound();
			
			event = *eventPos;
			switch(event){
			case 0x50: // PSG Write
				SndS.InitBlock_Begins(eventPos);
				continue;
			case 0x52: case 0x53: // YM2612
				SndS.QWrite(event & 1, eventPos);
				continue;
			case 0x80 ... 0x8f: // Dac Write
				SndS.QDacWrite(eventPos);
				continue;
			case 0xe0:
				SndS.QDacSeek(eventPos);
				continue;
			case 0x66:
				break;
			default:
				if(vgmPos.delay) 
					SndS.InitBlock_Ends(eventPos);
				continue;
			}
			break;
		}}
		
	printf("%X, %X\n", SndS.InitBlock_Start-vgmInfo.mainData, 
		SndS.InitBlock_End-vgmInfo.mainData);
	
	// pass2 variables
	SampScale curSamp(sscale);
	int loopSamp = 0;
	VgmPos vgmPos(vgmInfo);
	SndS.Init2(dupRemove, options & writeInitB);
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
			char* eventPos = vgmPos.next();

			if(SndS.InitBlock_Ends(eventPos, dupRemove))
				Codec.initBlock(curSamp, SndS.PreLoop_Dac, SndS.PreLoop_Seek);
			
			if(vgmPos.loop()) {
				loopSamp = curSamp;
				Codec.Flush(curSamp);
				vgmInfo.loopIndex = outData.curIndex();

				// Deal with initBlock
				if(SndS.LoopFound()) {
					Codec.initBlock(curSamp, 
						SndS.PostLoop_Dac, SndS.PostLoop_Seek);
				}
			}
			
			event = *eventPos;
			switch(event){
			case 0x50: // PSG Write
				Codec.eventWrite(curSamp, EventPSGP, (unsigned char*)eventPos+1);
				continue;
			case 0x52: case 0x53: { // YM2612
					int port = event & 1;
					if(SndS.Write(port, eventPos))
						Codec.eventWrite(curSamp, EventYMP0+port, (unsigned char*)eventPos+1);
					continue;}
			case 0xe0: // Dac seek
				if(SndS.Write_Dacs)
				Codec.eventWrite(curSamp, EventSEEK, (unsigned char*)eventPos+1);
				continue;
			case 0x4f: // Psg Stereo
				Codec.eventWrite(curSamp, EventPSGS, (unsigned char*)eventPos+1);
				continue;
			case 0x66: // Vgm end
				Codec.eventWrite(curSamp, Event_END, (unsigned char*)eventPos+1);
				Codec.Flush(curSamp);
				break;
			case 0x80 ... 0x8f: // Dac Write
				if(SndS.InitState != 0){	
					if(SndS.Write_Dacs)
					Codec.dacWrite(curSamp);
					Codec.Flush(curSamp);
				}else
				Codec.dacWrite(curSamp);
				
				goto CHECK_DELAY;

				// V1.60 dac stream events
			case 0x90 ... 0x95:
				Codec.dsWrite(curSamp, (unsigned char*)eventPos+1);
				continue;

			default:
			CHECK_DELAY:
				if(vgmPos.delay) {
					if((vgmPos.delay == 735)
					||(vgmPos.delay == 882))
						Codec.Flush(curSamp);
					curSamp += vgmPos.delay;
				}
				continue;
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
