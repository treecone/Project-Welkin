#pragma once
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <stb_image.h>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "Vertex.h"
#include "Material.h"
#include "PBRMaterial.h"
#include "Helper.h"
#include "Mesh.h"

namespace fs = std::filesystem;

struct Texture
{
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	VkDeviceSize imageSize;

	Texture(string PATH)
	{
		stbi_uc* pixels = stbi_load(PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load" + PATH + " texture image!");
		}
	}

	~Texture()
	{
		delete pixels;
		pixels = nullptr;
	}
};

class FileManager
{
public:
	void Init(VkDevice* device);
	std::unordered_map<std::string, VkShaderModule> allShaders;
	void LoadAllShaders(VkDevice* logicalDevice);
	~FileManager();


	#pragma region Models
	void LoadAllModels();
	Mesh* GetModel(string name);
	std::unordered_map<std::string, Mesh*> allMeshes;
	#pragma endregion

	#pragma region Materials
	std::vector<Material*> allMaterials;
	std::unordered_map<string, Texture*> allTextures;
	void CreateMaterial(string folderMaterialName);
	#pragma endregion



private:
	VkDevice* device;
	static std::vector<char> ReadFile(const std::string& filename);
	void LoadAllTextures(VkDevice* device);
	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* device);
};