#include "FileManager.h"

FileManager::FileManager()
{

}

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
    //CreateMaterial("Brick");
    CreateMaterial("VikingRoom");

    LoadAllShaders(device);
}

FileManager::~FileManager()
{
    for (const auto& shaderFile : allShaders)
    {
        vkDestroyShaderModule(*device, shaderFile.second, nullptr);
    }

    allMaterials.clear();
    allTextures.clear();
}

Mesh* FileManager::FindMesh(string name)
{
    unordered_map<string, unique_ptr<Mesh>>::const_iterator iter = allMeshes.find(name);

    if (iter != allMeshes.end())
    {
        // iter is item pair in the map. The value will be accessible as `iter->second`.
        return iter->second.get();
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

VkShaderModule FileManager::FindShaderModule(string name)
{
    unordered_map<string, VkShaderModule>::const_iterator iter = allShaders.find(name);

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
    Helper::Cout("- Loading All Shaders");

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
            pair<string, Mesh*> newMesh (rawName, new Mesh(path+fileName, device));
            allMeshes.insert(newMesh);
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

//TODO Make it so CreateMaterial loads texture while adding to material, right now there is a uneccesary loop 

void FileManager::CreateMaterial(string folderMaterialName)
{
    Helper::Cout("");
    Helper::Cout("-Loading Material for " + folderMaterialName);

    int numberOfTextures = 0;
    string fileName;

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

    //Remove first letter of the name
    fileName.erase(0, 1);

    Texture* foundTexColor = allTextures.find("c" + fileName)->second;
    if (foundTexColor == nullptr)
    {
        throw std::runtime_error("Found no color texture for " + fileName);
    }

    Helper::Cout("-- Loaded Texture for Material: " + fileName);


    if (numberOfTextures > 2)
    {
        Helper::Cout("-Creating [PBR] Material: " + fileName);

        //PBR material
        throw std::runtime_error("Created Material with no textures in it!");

        Texture* foundTexRoughness = allTextures.find("r" + fileName)->second;
        Texture* foundTexAO = allTextures.find("a" + fileName)->second;
        Texture* foundTexDepth = allTextures.find("d" + fileName)->second;
        Texture* foundTexNormal = allTextures.find("n" + fileName)->second;

        pair<string, Material*> newMaterial(fileName, new PBRMaterial(foundTexColor, fileName, this->device, foundTexRoughness, foundTexAO, foundTexDepth, foundTexNormal));
        allMaterials.insert(newMaterial);
    }
    else if (numberOfTextures > 0)
    {
        Helper::Cout("-Created [Normal] Material: " + fileName);
        pair<string, Material*> newMaterial(fileName, new Material(foundTexColor, fileName, this->device));
        allMaterials.insert(newMaterial);
    }

}

#pragma endregion
