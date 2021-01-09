#!/usr/bin/env python2.7

lenTable = [0] * 256

def build_table(fp, data):
	fp.write("static const char vgmLen[256] = {\n")
	index = 0
	for y in range(16):
		fp.write("\t/* %02X */ " % (index))
		for x in range(16):
			fp.write("0x%02X," % data[index])
			index += 1
		fp.write("\n")
	fp.write("};")
		
def setVal(addr, val, delay = False):
	if not delay: val |= 0x80
	lenTable[addr] = val

def setRange(beg, end, val, delay = False):
	for x in range(beg, end):
		setVal(x, val, delay)

# basic events
setRange(0x30, 0x40, 2)
setRange(0x40, 0x4E, 3)
setRange(0x4F, 0x51, 2)
setRange(0x51, 0x60, 3)
setRange(0xA0, 0xC0, 3)
setRange(0xC0, 0xDF, 4)
setRange(0xE0, 0xFF, 5)

# special events
setVal(0x61, 3, True)
setRange(0x62, 0x63, 1, True)
setVal(0x66, 1)
setRange(0x70, 0x80, 1, True)
setRange(0x80, 0x90, 1)

# dac stream events
setVal(0x90, 5)
setVal(0x91, 5)
setVal(0x92, 6)
setVal(0x93, 11)
setVal(0x94, 2)
setVal(0x95, 5)
	

fp = open("VgmLen.h", "w")
build_table(fp, lenTable)
