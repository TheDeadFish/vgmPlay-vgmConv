#!/usr/bin/env python2.7

regTable = [0] * 512

def build_table(fp, data):
	fp.write("static const char ymRegs[512] = {\n")
	index = 0
	for y in range(32):
		fp.write("\t/* %03X */ " % (index))
		for x in range(16):
			fp.write("0x%02X," % data[index])
			index += 1
		fp.write("\n")
	fp.write("};")

def setVal(port, addr, data):
	regTable[port*256 + addr] |= 0x80|data
	
# global registers
setVal(0, 0x22, 3)
setVal(0, 0x27, 3)
setVal(0, 0x28, 7)
setVal(0, 0x2A, 7)
setVal(0, 0x2B, 7)

# channel registers
for addr in range(0x30, 0xB8):
	if (addr & 3) == 3: continue
	if (addr & 0xF8) == 0xA8:
		setVal(0, addr, 2)
	else:
		setVal(0, addr, addr & 3)
		setVal(1, addr, 4 + (addr & 3))
		



fp = open("YmRegs.h", "w")
build_table(fp, regTable)