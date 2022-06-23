#include "Helper.h"

void Helper::Cout(std::string message, bool header)
{

#ifdef _DEBUG
	if (header)
	{
		std::cout << std::endl;
		std::cout << "---------------" + message + "---------------" << std::endl;
		std::cout << std::endl;
	}
	else
	{
		std::cout <<" " + message << std::endl;
	}
#endif
}
