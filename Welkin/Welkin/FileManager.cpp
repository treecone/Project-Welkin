#include "FileManager.h"

void FileManager::Init(VkDevice* device)
{
    Helper::Cout("File Manager", true);
    
    try
    {
        this->device = device;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    LoadAllModels();
    LoadAllTextures(device);
    CreateMaterial("Brick");
    CreateMaterial("Viking");

    LoadAllShaders(device);
}

FileManager::~FileManager()
{
    for (const auto& shaderFile : allShaders)
    {
        vkDestroyShaderModule(*device, shaderFile.second, nullptr);
    }

    allMeshes.clear();

    for (auto& material : allMaterials)
    {
        delete material;
        material = nullptr;
    }

    allTextures.clear();
}

#pragma region Shaders

void FileManager::LoadAllShaders(VkDevice* logicalDevice)
{
    Helper::Cout("-Loading Shaders");
    Helper::Cout("[Note] Maybe make it so it doesn't load all shaders?");

    std::string path = "Shaders/";
    std::string ext = { ".spv" };
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        std::string fileName = entity.path().filename().string();
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                allShaders.insert({ fileName, CreateShaderModule(ReadFile(path + fileName), logicalDevice) });
                Helper::Cout("-- Loaded Shader: " + fileName);
            }
        }
    }

    Helper::Cout("All Shaders Loaded!");
}

VkShaderModule FileManager::CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* logicalDevice)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> FileManager::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

#pragma endregion

#pragma region Models

void FileManager::LoadAllModels()
{
    std::string path = "Models/";
    std::string ext = { ".obj" };
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        std::string fileName = entity.path().filename().string();
        if (fs::is_regular_file(entity))
        {
            pair<string, Mesh*> newMesh (fileName, new Mesh(entity.path().string(), device));
            allMeshes.insert(newMesh);
        }
    }
}

Mesh* FileManager::GetModel(string name)
{
    return allMeshes.find(name)->second;
}

#pragma endregion

#pragma region Materials

//First load all textures, then create the materials with pointers to the appropriate textures

void FileManager::LoadAllTextures(VkDevice* logicalDevice)
{
    Helper::Cout("-Loading all textures");

    string path = "Materials";
    string ext = { ".png" };
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        string fileName = entity.path().filename().string();
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                pair<string, Texture*> newTex(fileName, new Texture(path));
                allTextures.insert(newTex);
                Helper::Cout("-- Loaded Texture: " + fileName);
            }
        }
    }
}

void FileManager::CreateMaterial(string folderMaterialName)
{
    int numberOfTextures;
    string fileName;

    string path = "Materials/" + folderMaterialName;
    string ext = { ".png" };
    for (auto& entity : fs::directory_iterator(path))
    {
        if(fileName == "")
            fileName = entity.path().filename().string();

        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                numberOfTextures++;
            }
        }
    }

    //Remove first letter of the name
    fileName.erase(0, 1);

    Texture* foundTexColor = allTextures.find("c" + fileName)->second;
    if (foundTexColor == nullptr)
    {
        throw std::runtime_error("Found no color texture for " + fileName);
    }

    if (numberOfTextures > 2)
    {
        Helper::Cout("-Creating PBR Material: " + fileName);

        //PBR material
        throw std::runtime_error("Created Material with no textures in it!");

        Texture* foundTexRoughness = allTextures.find("r" + fileName)->second;
        Texture* foundTexAO = allTextures.find("a" + fileName)->second;
        Texture* foundTexDepth = allTextures.find("d" + fileName)->second;
        Texture* foundTexNormal = allTextures.find("n" + fileName)->second;


        allMaterials.push_back(new PBRMaterial(foundTexColor, fileName, this->device, foundTexRoughness, foundTexAO, foundTexDepth, foundTexNormal));
    }
    else if (numberOfTextures > 0)
    {
        Helper::Cout("-Creating Material: " + fileName);

        allMaterials.push_back(new Material(foundTexColor, fileName, this->device));
    }

}

#pragma endregion
