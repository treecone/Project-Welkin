#include "Helper.h"

void Helper::Cout(std::string message, bool header)
{

#ifdef _DEBUG
	if (header)
	{
		std::cout << "\n --------------- " + message + " --------------- \n" << std::endl;
	}
	else
	{
		std::cout <<" " + message << std::endl;
	}
#endif
}

void Helper::Warning(std::string message)
{
#ifdef _DEBUG
	std::cout << "\n[[WARNING]]: " + message + "\n" << std::endl;
#endif
}

float Helper::Q_rsqrt(float number)
{
	//https://en.wikipedia.org/wiki/Fast_inverse_square_root
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y));

#ifndef Q3_VM
#ifdef __linux__
	assert(!isnan(y));
#endif
#endif
	return y;
}