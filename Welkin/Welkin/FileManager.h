#pragma once
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>
#include <fstream>
#include "Texture.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include "Vertex.h"
#include "Material.h"
#include "PBRMaterial.h"
#include "Helper.h"
#include "Mesh.h"

namespace fs = std::filesystem;

class FileManager
{
public:
	FileManager();
	~FileManager();

	void Init(VkDevice* device);


	Mesh* FindMesh(string name);
	Material* FindMaterial(string name);
	VkShaderModule FindShaderModule(string name);


private:

	VkDevice* device;

	void LoadAllTextures(VkDevice* logicalDevice);
	std::pair<string, unsigned short> LoadTexturesFromFolder(string folderName);
	void LoadAllModels();
	void CreateMaterial(string folderMaterialName, bool loadTexturesFromFolder = true);

	void LoadAllShaders(VkDevice* logicalDevice);
	VkShaderModule CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* device);
	static std::vector<char> ReadFile(const std::string& filename);
	

	//TODO switch these to unique pointers 
	std::unordered_map<string, unique_ptr<Mesh>> allMeshes;
	std::unordered_map<std::string, VkShaderModule> allShaders;
	std::unordered_map<string, unique_ptr<Material>> allMaterials;
	std::unordered_map<string, unique_ptr<Texture>> allTextures;
};