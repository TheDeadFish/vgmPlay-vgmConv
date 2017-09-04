#include "vgmConv.hpp"
#include "codec/codecs.hpp"
#include "codec/vgcEvents.h"
#include <NextGet.hpp>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <new>
using namespace std;

int VgmConv::vgcConvert(vgcFile& vgcInfo)
{
	// Main variables
	unsigned char event;
	int curSamp = 0;
	int loopSamp = 0;
	bool loopFound = false;
	MemWrite outData;
	codec Codec;
	char *loopIndexP;
	
	// Setup loopIndexP
	if(vgcInfo.loopIndex != 0xffffffff){
		loopIndexP = vgcInfo.mainData + vgcInfo.loopIndex;
		//printf("%x\n", vgcInfo.loopIndex);
		loopFound = true;
	}else
		loopIndexP = NULL;
	
	// setup variables
	std::unique_ptr<eventCodec> EC(new (nothrow) vgmEvent());
	std::unique_ptr<dacCodec> DC(new (nothrow) vgmDac());
	if((EC.get() == NULL)||(DC.get() == NULL))
		return VGX_MEM_ERR;
		
	Codec.EC = EC.get();
	Codec.DC = DC.get();
	Codec.Init(&outData);
		
	
	// Setup loop
	jmp_buf jmpBuf;
	//outData.SetExcp(&jmpBuf, 1);
	NextGet vgcPos(
		vgcInfo.mainData, vgcInfo.mainSize,	jmpBuf, 2);
	switch(setjmp( jmpBuf ))
	{
	case 1:
		return VGX_MEM_ERR;
	case 2:
		return VGX_FILE_BAD;
	}
	
	// begin loop
	for(;;){
		// Check loop
		if(loopFound){
			if(vgcPos.curPos > loopIndexP)
				return VGX_FILE_BAD;
			if(vgcPos.curPos == loopIndexP){
				loopSamp = curSamp;
				Codec.Flush(curSamp);
				vgcInfo.loopIndex = outData.curIndex();
				loopFound = false;
			}
		}
		
		event = vgcPos.Get<char>();
		switch(event & 0xf0){
			// Delay Events
			case VGC_N:
				curSamp += (event & 0x0f);
				curSamp += 1;
				continue;
			case VGC_NNN:{
				int tmp = (event & 0x0f);
				tmp |= (vgcPos.Get<unsigned char>() << 4);
				curSamp += tmp;
				continue;}
			case VGC_NNNN:{	
				int tmp = (vgcPos.Get<unsigned char>() << 8);
				tmp |= vgcPos.Get<unsigned char>();
				curSamp += tmp;
				continue;}
			
			// Normal Events
			case VGC_YMP0:
				vgcPos.Range(2);
				Codec.eventWrite(curSamp, EventYMP0, (unsigned char*)vgcPos.curPos);
				vgcPos.Add(2);
				curSamp += (event & 0x0f);
				continue;
			case VGC_YMP1:
				vgcPos.Range(2);
				Codec.eventWrite(curSamp, EventYMP1, (unsigned char*)vgcPos.curPos);
				vgcPos.Add(2);
				curSamp += (event & 0x0f);
				continue;
			case VGC_PSGP:
				vgcPos.Range(1);
				Codec.eventWrite(curSamp, EventPSGP, (unsigned char*)vgcPos.curPos);
				vgcPos.Add(1);
				curSamp += (event & 0x0f);
				continue;
			case VGC_SEEK:
				vgcPos.Range(3);
				unsigned char ptmp[4];
				ptmp[0] = vgcPos.curPos[2]; ptmp[1] = vgcPos.curPos[1]; 
				ptmp[2] = vgcPos.curPos[0]; ptmp[3] = 0;
				Codec.eventWrite(curSamp, EventSEEK, &ptmp[0]);
				vgcPos.Add(3);
				curSamp += (event & 0x0f);
				continue;
				
			// Dac Events
			case VGC_DAC1_1:
				Codec.dacWrite(curSamp);
				curSamp += (event & 0x0f);
				continue;
			case VGC_AvDac:{
				int big = event & 0x0f;
				int size = vgcPos.Get<unsigned char>();
				int sma = vgcPos.Get<unsigned char>();
				int slebus = 0;
				while(size-- >= 0){
					Codec.dacWrite(curSamp);
					curSamp += big;
					slebus -= sma;
					if(slebus <= 0){
						slebus += 256;
						curSamp += 1;
					}
				}
				continue;
			}
			case VGC_DAC1_2:{
				int count = int(event & 0x0f) + 2;
				while(count--){
					unsigned char c = vgcPos.Get<char>();
					Codec.dacWrite(curSamp);
					curSamp += (c & 0x0f);
					Codec.dacWrite(curSamp);
					curSamp += (c >> 4);
				}
				continue;
			}
			case VGC_DAC1_4:
			case VGC_DAC1_8:{
				int size = ((event & 0xf0) == VGC_DAC1_4) ? 3 : 1;
				int andVal = ((event & 0xf0) == VGC_DAC1_4) ? 3 : 7;
				int count = vgcPos.Get<unsigned char>();
				do{
					unsigned char c = vgcPos.Get<char>();
					do{
						Codec.dacWrite(curSamp);
						curSamp += ((event & 0x0f) + (c & size));
						if(size == 3)
							c >>= 1;
						c >>= 1;
					}while(count-- & andVal);
				}while(count > 0);
				continue;
			}
		
			// Other Events
			default:
				switch(event){
					case VGC_END:
						Codec.eventWrite(curSamp, Event_END, (unsigned char*)vgcPos.curPos);
						Codec.Flush(curSamp);
						break;
					case VGC_PSGS:
						vgcPos.Range(1);
						Codec.eventWrite(curSamp, EventPSGS, (unsigned char*)vgcPos.curPos);
						vgcPos.Add(1);
						continue;
					case VGC_60th:
						curSamp += 735;
						continue;
					case VGC_50th:
						curSamp += 882;
						continue;
					default:
						// Non supported, error
						return VGX_FILE_BAD;
				}
				break;
		}
		break;
	}
	
	// set up vgmInfo structure and save data
	vgmFile vgmInfo;
	vgmInfo.header = NULL;
	vgmInfo.mainData = outData.getBase();
	vgmInfo.mainSize = outData.curIndex();
	vgmInfo.loopIndex = vgcInfo.loopIndex;
	vgmInfo.sampData = vgcInfo.sampData;
	vgmInfo.sampSize = vgcInfo.sampSize;
	vgmInfo.gd3Data = vgcInfo.gd3Data;
	vgmInfo.gd3Size = vgcInfo.gd3Size;
	vgmInfo.samples = curSamp;
	vgmInfo.loopSamp = curSamp - loopSamp;
	if(dest == NULL)
		fileData = vgmInfo.Save(fileSize);
	else
		vgmInfo.Save(dest, options & compressO);
	return vgmInfo.status;
}
