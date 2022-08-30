#pragma once
#include <vector>
#include <string>
#include "Helper.h"
#include<map>
#include <filesystem>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <fstream>

namespace fs = std::filesystem;

struct imageContainer
{
	stbi_uc* image;
	int texWidth, texHeight, texChannels;

	~imageContainer()
	{

	}
};

class FileManager
{
public:
	void Init(VkDevice* device);
	std::map<std::string, VkShaderModule> allShaders;
	std::vector<imageContainer>  allTextures;
	void LoadAllShaders(VkDevice* logicalDevice);
	void LoadAllModels();
	void LoadAllTexture();
	~FileManager();

private:
	VkDevice* device;
	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* device);
};