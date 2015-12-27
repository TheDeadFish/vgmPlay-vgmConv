#ifndef _VGCEVENTS_H_
#define _VGCEVENTS_H_
// Bytes codes for vgc format

// Delay Events
#define VGC_N 		0x00
#define VGC_NNN 	0x10
#define VGC_NNNN	0x20

// Normal Events
#define VGC_YMP0	0x30
#define VGC_YMP1	0x40
#define VGC_PSGP	0x50
#define VGC_SEEK	0x60

// Dac Events
#define VGC_DAC1_1	0xE0
#define VGC_DAC1_2	0xD0
#define VGC_DAC1_4	0xC0
#define VGC_DAC1_8	0xB0
#define VGC_AvDac 	0xA0

// Other Events
#define VGC_END 	0xF0
#define VGC_PSGS	0xF1
#define VGC_60th	0xF2
#define VGC_50th	0xF3

#endif
