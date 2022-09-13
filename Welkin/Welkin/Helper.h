#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.h>


struct Helper
{
	static void Cout(std::string message, bool header = false);
	static void Warning(std::string message);
	float Q_rsqrt(float number);
};