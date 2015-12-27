#ifndef _SAMPSCALE_H_
#define _SAMPSCALE_H_

class SampScale
{
	double data;
	double scale;
public:
	SampScale(double in);
	void operator+=(int addr);
	operator int();
};

#endif
