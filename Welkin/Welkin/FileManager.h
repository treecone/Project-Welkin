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
#include "Helper.h"
#include "Mesh.h"


namespace fs = std::filesystem;



class FileManager
{
public:
	void Init(VkDevice* device);
	std::unordered_map<std::string, VkShaderModule> allShaders;
	void LoadAllShaders(VkDevice* logicalDevice);
	void LoadAllTexture();
	~FileManager();

	#pragma region Models
	void LoadAllModels();
	std::unordered_map<std::string, Mesh> allMeshes; //TODO ask caden
	#pragma endregion


	#pragma region Textures

	struct ImageContainer
	{
		stbi_uc* image;
		int texWidth, texHeight, texChannels;

		~ImageContainer()
		{
			delete image;
			image = nullptr;
		}
	};

	std::vector<ImageContainer>  allTextures;
	#pragma endregion


	


private:
	VkDevice* device;
	static std::vector<char> ReadFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* device);
};