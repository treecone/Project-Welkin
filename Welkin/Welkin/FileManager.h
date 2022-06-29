#pragma once
#include <vector>
#include <string>
#include "Helper.h"
#include<map>
#include <filesystem>
#include <vulkan/vulkan.h>
#include <fstream>

namespace fs = std::filesystem;

class FileManager
{
public:
	FileManager();
	std::map<std::string, VkShaderModule> allShaders;
	void LoadAllShaders(VkDevice* logicalDevice);

private:
	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* device);
};

