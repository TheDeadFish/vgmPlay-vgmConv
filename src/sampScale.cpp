#include "sampScale.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

SampScale::SampScale(double in)
{
	data = 0;
	scale = in;
}

void SampScale::operator+=(int addr)
{
	data += scale * addr;
}

SampScale::operator int()
{
	return lrint(data);
}
