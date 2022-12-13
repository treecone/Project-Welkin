#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


namespace Welkin_Settings
{
	static const unsigned int MAX_OBJECTS = 2048;
};

namespace Welkin_BufferStructs
{
	struct PerTransformStruct
	{
		alignas(16) glm::mat4 world;
		alignas(16) glm::mat4 worldInverseTranspose;
	};

	struct PushConstant
	{
		//aka model->world matrix
		alignas(16) int tempData;

		//unsigned short materialID;
	};
};


namespace Helper
{
	void Cout(std::string message, bool header = false);
	void Warning(std::string message);
	float Q_rsqrt(float number);
};

namespace vHelper
{

};