#include "Vgc.hpp"
#include "Vgx.hpp"

class vgc_header
{
public:
	int magic;
	int version;
	int start;
	int loop;
	int gd3;
	int size;
	int image;
	int sampSize;
		
	int getStart(void){
		return SwapEndian(start);}
		
	void setStart(int in){
		start = SwapEndian(in);}
		
	int getLoop(void){
		return SwapEndian(loop);}
		
	void setLoop(int in){
		loop = SwapEndian(in);}
	
	int getGd3(void){
		return SwapEndian(gd3);}
	
	void setGd3(int in){
		gd3 = SwapEndian(in);}
		
	int getSize(void){
		return SwapEndian(size);}
		
	void setSize(int in){
		size = SwapEndian(in);}
		
	int getSampSize(void){
		return SwapEndian(sampSize);}
		
	void setSampSize(int in){
		sampSize = SwapEndian(in);}

private:
	static int SwapEndian(int in);
};

int vgc_header::SwapEndian(int in)
{
	union{int var;char ary[4];
	}in1, out;
	in1.var = in;
	out.ary[0] = in1.ary[3];
	out.ary[1] = in1.ary[2];
	out.ary[2] = in1.ary[1];
	out.ary[3] = in1.ary[0];
	return(out.var);
}

bool VgcFile_::Load(vgx_fileIo& fp)
{
	// Error handling macros
	#define FP_READ(data, size){	\
		if(! fp.Read(data, size))	\
			return false;}
	
	// Get header of vgc file
	vgc_header head;
	FP_READ(&head.version, sizeof(head) - 4);
	
	// Check version  number
	if(head.version != 0x00010000)
	{
		fp.FileBad();
		return false;
	}
		
	// Allocate buffer for file
	int vgcSize = head.getSize();
	if(! buffer.alloc(vgcSize))
	{
		fp.AllocErr();
		return false;
	}
	header = (vgc_header*)buffer;
	
	// Move head to vgc buffer
	memcpy(header, &head, sizeof(head));
	
	// Read rest of vgc data
	int sizeRemain = vgcSize - sizeof(head);
	FP_READ(buffer + sizeof(head), sizeRemain);
	
	// Get pointer and size of sample block
	int mainIndex = head.getStart();
	if(head.getSampSize())
	{
		sampSize = head.getSampSize();
		sampData = buffer + mainIndex + 4;
		mainIndex += sampSize;
	}else
		sampData = NULL;
	mainIndex += 4;
	
	// Get pointer and size of vgc data
	mainData = buffer + mainIndex;
	if(head.getGd3())
		mainSize = head.getGd3() - mainIndex;
	else
		mainSize = vgcSize - mainIndex;
	
	// Get loop index
	if(head.getLoop()){
		loopIndex = head.getLoop();
		loopIndex -= mainIndex;
	}
	else
		loopIndex = 0xffffffff;
		
	// Get Pointer and size of gd3 data
	if(head.getGd3()){
		int gd3offset = head.getGd3();
		gd3Data = buffer + gd3offset;
		gd3Size = vgcSize - gd3offset;
	}else
		gd3Data = NULL;
	return true;
}

bool VgcFile_::Save(vgx_fileIo& fp)
{
	// Error handling macros
	#define FP_WRITE(data, size){	\
		if(! fp.Write(data, size))	\
			return false;}
	
	// create header
	vgc_header head = {};
	head.magic = 0x20636756;
	head.version = 0x00010000;
	head.setStart(sizeof(head)-4);
	
	// dac offset
	int curPos = sizeof(head);
	if(sampData != NULL){
		head.setSampSize(sampSize);
		curPos += sampSize;
	}
	
	// loopPoit offset
	if(loopIndex != 0xffffffff){
		head.setLoop(loopIndex + curPos);
	}
	curPos += mainSize;
	
	// gd3 offset
	if(gd3Data != NULL){
		head.setGd3(curPos);
		curPos += gd3Size;
	}
	
	// eof offset
	head.setSize(curPos);
	
	// Reserve space in file
	if(! fp.Reserve(curPos))
		return false;
	
	// Write vgm header to file
	FP_WRITE(&head, sizeof(head));
	
	// write dac samples to file
	if(sampData != NULL)
		FP_WRITE(sampData, sampSize);
		
	// write main data to file
	FP_WRITE(mainData, mainSize);
	
	// write gd3data to file
	if(gd3Data != NULL)
		FP_WRITE(gd3Data, gd3Size);
	return true;
}

int vgcFile_setSize(void* ptr, int size)
{
	vgc_header* head = (vgc_header*)ptr;
	int oldSize = head->getSize();
	head->setSize(size); return oldSize;
}
