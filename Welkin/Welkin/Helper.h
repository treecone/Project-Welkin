#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


namespace Helper
{
	void Cout(std::string message, bool header = false);
	void Warning(std::string message);
	float Q_rsqrt(float number);
};

namespace vHelper
{
	struct PushConstants
	{
		//TODO transfer to storage buffer 
		//aka model->world matrix
		alignas(16) glm::mat4 world; //64 bits
		alignas(16) glm::mat4 worldInverseTranspose;

		//unsigned short materialID;
	};
};