#pragma once
#include <iostream>
#include <string>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


namespace Welkin_Settings
{
	static const unsigned int MAX_OBJECTS = 2048;
};

enum TEXTURE_TYPE
{
	TT_AMBIENTOCCLUSION = 0,
	TT_COLOR = 1,
	TT_DEPTH = 2,
	TT_NORMAL = 3,
	TT_ROUGHNESS = 4
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
		alignas(4) unsigned int instanceID;
		alignas(4) unsigned int materialID;
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