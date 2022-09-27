#include "FileManager.h"
#include "VulkanCore.h"

FileManager::FileManager()
{

}

void FileManager::Init(VulkanCore* vCore)
{
    Helper::Cout("File Manager", true);
    this->vCore = vCore;
    this->device = vCore->GetLogicalDevice();
    
    try
    {
        this->device = device;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    LoadAllModels();
    //LoadAllTextures(device);
    //CreateMaterial("Brick");
    CreateMaterial("VikingRoom");;

    LoadAllShaders(device);
}

FileManager::~FileManager()
{
    for (const auto& shaderFile : allShaders)
    {
        vkDestroyShaderModule(*device, *shaderFile.second, nullptr);
        delete shaderFile.second;
    }

    for (const auto& meshFile : allMeshes)
    {
        delete meshFile.second;
    }

    for (const auto& materialFile : allMaterials)
    {
        delete materialFile.second;
    }

    for (const auto& textureFile : allTextures)
    {
        delete textureFile.second;
    }
}

Mesh* FileManager::FindMesh(string name)
{
    unordered_map<string, Mesh*>::const_iterator iter = allMeshes.find(name);

    if (iter != allMeshes.end())
    {
        // iter is item pair in the map. The value will be accessible as `iter->second`.
        return iter->second;
    }

    Helper::Cout("[Warning] Couldn't find mesh: " + name);
    return nullptr;
}

Material* FileManager::FindMaterial(string name)
{
    unordered_map<string, Material*>::const_iterator iter = allMaterials.find(name);

    if (iter != allMaterials.end())
    {
        // iter is item pair in the map. The value will be accessible as `iter->second`.
        return iter->second;
    }

    Helper::Cout("[Warning] Couldn't find material: " + name);
    return nullptr;
}

VkShaderModule* FileManager::FindShaderModule(string name)
{
    unordered_map<string, VkShaderModule*>::const_iterator iter = allShaders.find(name);

    if (iter != allShaders.end())
    {
        // iter is item pair in the map. The value will be accessible as `iter->second`.
        return iter->second;
    }

    throw std::runtime_error("Couldn't find shader: " + name);
}


#pragma region Shaders

//TODO Maybe make it so it doesn't load all shaders?
void FileManager::LoadAllShaders(VkDevice* logicalDevice)
{
    Helper::Cout("");
    Helper::Cout("Loading All Shaders");

    string path = "Shaders/";
    string ext = { ".spv" };
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        string fileName = entity.path().filename().string();
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                std::pair<string, VkShaderModule*> newPair(fileName, new VkShaderModule());
                *newPair.second = CreateShaderModule(ReadFile(path + fileName), logicalDevice);
                allShaders.insert(newPair);
                Helper::Cout("- Loaded Shader: " + fileName);
            }
        }
    }

    Helper::Cout("- All Shaders Loaded!");
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
        size_t lastIndex = fileName.find_last_of(".");
        string rawName = fileName.substr(0, lastIndex);
        if (fs::is_regular_file(entity))
        {
			if (entity.path().extension() == ext)
			{
				pair<string, Mesh*> newMesh(rawName, new Mesh(path + fileName, vCore));
				allMeshes.insert(newMesh);
			}
        }
    }
}
#pragma endregion

#pragma region Materials

//First load all textures, then create the materials with pointers to the appropriate textures

void FileManager::LoadAllTextures(VkDevice* logicalDevice)
{
    Helper::Cout("-Loading all textures");

    string path = "Materials/";
    string ext = { ".png" };
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        string fileName = entity.path().filename().string();
        size_t lastIndex = fileName.find_last_of(".");
        string rawName = fileName.substr(0, lastIndex);
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                pair<string, Texture*> newTex(rawName, new Texture(entity.path().string()));
                allTextures.insert(newTex);
                Helper::Cout("-- Loaded Texture: " + fileName);
            }
        }
    }
}

//Returns first raw file name and number of textures found
std::pair<string, unsigned short> FileManager::LoadTexturesFromFolder(string folderName)
{
    Helper::Cout("-Loading all textures from folder: " + folderName);
    int numberOfTextures = 0;
    string firstFileName = "";

    string path = "Materials/" + folderName;
    string ext = { ".png" };
    for (auto& entity : fs::directory_iterator(path))
    {
        string fileName = entity.path().filename().string();
        size_t lastIndex = fileName.find_last_of(".");
        string rawName = fileName.substr(0, lastIndex);
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                pair<string, Texture*> newTex(rawName, new Texture(entity.path().string()));
                allTextures.insert(newTex);
                Helper::Cout("-- Loaded Texture: " + fileName);

                numberOfTextures++;
                if (firstFileName == "")
                {
                    firstFileName = rawName;
                }
            }
        }
    }

    return std::pair<string, unsigned short>(firstFileName, numberOfTextures);
}

void FileManager::CreateMaterial(string folderMaterialName, bool loadTexturesFromFolder)
{
    Helper::Cout("");
    Helper::Cout("Creating Material for " + folderMaterialName);

    string fileName = "";
    int numberOfTextures = 0;

    if (loadTexturesFromFolder)
    {
        std::pair<string, unsigned short> loadedTextures = LoadTexturesFromFolder(folderMaterialName);
        fileName = loadedTextures.first;
        numberOfTextures = loadedTextures.second;
    }
    else
    {
        //Find how many textures so we can create the right type of material
        //Assuming there are only two types, normal (1-2) or PBR (> 2)

        string path = "Materials/" + folderMaterialName;
        string ext = { ".png" };
        for (auto& entity : fs::directory_iterator(path))
        {
            if (fileName == "")
            {
                fileName = entity.path().filename().string();
                size_t lastIndex = fileName.find_last_of(".");
                fileName = fileName.substr(0, lastIndex);
            }

            if (fs::is_regular_file(entity))
            {
                if (entity.path().extension() == ext)
                {
                    numberOfTextures++;
                }
            }
        }

    }
    
    //Remove first letter of the name to get the generic name of all the textures
    fileName.erase(0, 1);

    Texture* foundColorTextureName = allTextures.at("c" + fileName);

    if (foundColorTextureName == nullptr)
    {
        Helper::Warning("Found no color texture for " + fileName + " --Skipping");
        return;
    }

    if (numberOfTextures > 2)
    {
        Helper::Cout("-Creating [PBR] Material: " + fileName);

        //PBR material
        throw std::runtime_error("Created Material with no textures in it!");

        Texture* foundTexRoughness = allTextures.at("r" + fileName);
        Texture* foundTexAO = allTextures.at("a" + fileName);
        Texture* foundTexDepth = allTextures.at("d" + fileName);
        Texture* foundTexNormal = allTextures.at("n" + fileName);

        pair<string, Material*> newMaterial(fileName, new PBRMaterial(foundColorTextureName, fileName, this->device, foundTexRoughness, foundTexAO, foundTexDepth, foundTexNormal, glm::vec2(1, 1)));
        allMaterials.insert(newMaterial);
    }
    else if (numberOfTextures > 0)
    {
        pair<string, Material*> newMaterial(fileName, new Material(foundColorTextureName, fileName, this->device, glm::vec2(1, 1)));
        allMaterials.insert(newMaterial);
        Helper::Cout("Created [Normal] Material: " + fileName);
    }
    else
    {
        Helper::Warning("Found no textures to create " + fileName + " material!");
    }

}

#pragma endregion
