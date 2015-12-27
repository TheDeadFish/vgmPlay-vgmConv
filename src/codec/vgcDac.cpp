#include "vgcDac.hpp"
#include "vgcEvents.h"

int vgcDac::rleSearch(int index, int max, int size)
{
	unsigned char byteMax;
	unsigned char byteMin;
	int minLeft = 32;
	if(size == 3)
		minLeft = 4;
	while(index + minLeft <= max){
		int maxTmp = max;
		if(max - index > 256)
			maxTmp = index + 256;
		byteMin = byteMax = cmd8n[index];
		int i;
		for(i = index+1; i < maxTmp; i++){
			unsigned char byteMaxTemp = byteMax;
			unsigned char byteMinTemp = byteMin;
			if(byteMaxTemp < (unsigned char)cmd8n[i])
				byteMaxTemp = (unsigned char)cmd8n[i];
			if(byteMinTemp > (unsigned char)cmd8n[i])
				byteMinTemp = (unsigned char)cmd8n[i];
			if(byteMaxTemp - byteMinTemp > size)
				break;
			byteMax = byteMaxTemp;
			byteMin = byteMinTemp;
		}
		if((i - index) >= minLeft){
			// Write output to class global variable
			if(size == 3){
				rle8_2Min = byteMin;
				rle8_2Index = index;
				rle8_2End = i;
			}else{
				rle8_1Min = byteMin;
				rle8_1Index = index;
				rle8_1End = i;
			}
			return(index);
		}
		index++;
	}
	if(size == 3){
		rle8_2End = 0;
	}else
		rle8_1End = 0;
	return(max);
}

void vgcDac::rleWrite(int size)
{
	int byteMin;
	int index;
	int i;
	if(size == 3){
		byteMin = rle8_2Min;
		index = rle8_2Index;
		i = rle8_2End;
	}else{
		byteMin = rle8_1Min;
		index = rle8_1Index;
		i = rle8_1End;
	}
	//printf("%d :", i - index);
	
	// output reduced range data
	if(size == 3){
		outData->Write<char>(VGC_DAC1_4 | (byteMin & 0x0f));
		//printf("%x ", (VGC_DAC1_4 | (byteMin & 0x0f)) & 255);
	}else{
		outData->Write<char>(VGC_DAC1_8 | (byteMin & 0x0f));
		//printf("%x ", (VGC_DAC1_8 | (byteMin & 0x0f)) & 255);
	}
	char tempByte=0;
	int bitPos=0;
	char *ptrCmd8n = &cmd8n[index];
	int count = (i - index) - 1;
	outData->Write<char>(count);
	//printf("%x ", count & 255);
	do{
		tempByte |= (((*ptrCmd8n++ - byteMin)) << bitPos);
		if(((size == 3)&&((count & 3) == 0))||
		(size != 3)&&((count & 7) == 0)){
			outData->Write<char>(tempByte);
			//printf("%d:%x ", count, tempByte & 255);
			tempByte = 0;
			bitPos = 0;
		}else{ 
			if(size == 3){
				bitPos += 2;
			}else
				bitPos += 1;
		}
	}while(count--);
	//printf("\n");
}

//extern int screbVar;

void vgcDac::rleFlush(void)
{
	if(mode == 1){
	// avSamp mode
	int CurOutPos = 0;
	while(idx8n - CurOutPos > 3){
		int size = idx8n - CurOutPos;
		if(size > 256) size = 256;
		// Calculate average
		float total = 0;
		for(int i = CurOutPos; i < (CurOutPos+size); i++)
			total += cmd8n[i]&15;
		total /= float(size);
		//printf("%d %f\n", size, total);
		// Output average encoded
		int temp = int(total*256)+1;
		outData->Write<char>(VGC_AvDac + (temp >> 8));
		outData->Write<char>(size-1);
		//outData->Write<char>(temp >> 8);
		outData->Write<char>(temp);
		CurOutPos += size;
	}
	while(idx8n - CurOutPos)
		outData->Write<char>(cmd8n[CurOutPos++]);
		idx8n = 0;
	
	}else{
	// normal mode
	int CurOutPos = 0;
	//if(screbVar >= 0x5E59F)
	while(CurOutPos < idx8n){
		int NewOutPos1 = rleSearch(CurOutPos, idx8n, 1);
		while(CurOutPos < NewOutPos1){
			int NewOutPos2 = rleSearch(CurOutPos, NewOutPos1, 3);
		
			// Output nibble encoded delays
			//if(screbVar >= 0x5E59F)
			//printf("nf: %d - %d \n", CurOutPos, NewOutPos2);
			while(CurOutPos < NewOutPos2){
				int diff = NewOutPos2 - CurOutPos;
				if(diff < 4)
					break;
				//printf("%d: ", diff);
				int runLen = (diff >> 1) - 1;
				if(runLen > 16) 
					runLen = 16;
				outData->Write<char>(VGC_DAC1_2 + (runLen-1));
				//printf("%x, ", VGC_DAC1_2 + (runLen-1));
				//if(screbVar >= 0x5E59F)
				//printf("n: %d\n", runLen + 1);
				do{
					outData->Write<char>((cmd8n[CurOutPos] & 0x0F) | ((cmd8n[CurOutPos + 1] & 0x0F) << 4));
					//printf("%x, ", (cmd8n[CurOutPos] & 0x0F) | ((cmd8n[CurOutPos + 1] & 0x0F) << 4));
					CurOutPos += 2;
				}while(runLen--);
				//printf("\n");
			}
			// Output remaining 0x8n
			while(CurOutPos < NewOutPos2){
				//if(screbVar >= 0x5E59F)
				//printf("%x, ", cmd8n[CurOutPos]);
				outData->Write<char>(cmd8n[CurOutPos++]);
			}
			//if(screbVar >= 0x5E59F)
			//printf("\n");
	
			// Output half nibble encoded delays
			//if(screbVar >= 0x5E59F)
			//printf("hn: %d %d\n", rle8_2Index, rle8_2End);
			if(rle8_2End){
				rleWrite(3);
				CurOutPos = rle8_2End;
			}
		}
		if(rle8_1End){
			rleWrite(1);
			CurOutPos = rle8_1End;
		}
	}
	idx8n = 0;
	//if(screbVar >= 0x5E59F)
		//printf("\n");
}}

void vgcDac::Init(int *curSampIn, MemWrite *outDataIn)
{
	curSamp = curSampIn;
	outData = outDataIn;
	writePend = false;
	idx8n = 0;
}

void vgcDac::Core(int targetSamp, bool pendState)
{
	if(writePend){
		bool doFlush = false;
		if(pendState == false)
			doFlush = true;
		int delay = targetSamp - *curSamp;
		if(delay > 15){
			doFlush = true;
			delay = 15;
		}
		cmd8n[idx8n++] = (VGC_DAC1_1 + delay);
		if(idx8n >= MAX_SAMP)
			doFlush = true;
		if(doFlush)
			rleFlush();
		*curSamp += delay;
	}
	if(targetSamp - *curSamp)
		*curSamp = vgcCodec_Delay(targetSamp, *curSamp, outData);
	writePend = pendState;
	//printf("vgcDac::Core; Out\n");
}

void vgcDac::Write(int targetSamp)
{
	Core(targetSamp, true);
}

void vgcDac::Flush(int targetSamp)
{
	Core(targetSamp, false);
}

void *vgcDac::Get(int *size)
{
	*size = 0;
	return((void*)0);
}
