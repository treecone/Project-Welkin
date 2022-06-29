#include "FileManager.h"

FileManager::FileManager()
{
    Helper::Cout("File Manager", true);
}

void FileManager::LoadAllShaders(VkDevice* logicalDevice)
{
    Helper::Cout("[File Manager]Loading Shaders");

    std::string path = "Shaders/";
    std::string ext = {".spv"};
    for (auto& entity : fs::recursive_directory_iterator(path))
    {
        std::string fileName = entity.path().filename().string();
        if (fs::is_regular_file(entity))
        {
            if (entity.path().extension() == ext)
            {
                allShaders.insert({fileName, CreateShaderModule(ReadFile(path + fileName), logicalDevice)});
                Helper::Cout("[File Manager]- Loaded Shader: " + fileName);
            }   
        }
    }

    Helper::Cout("[File Manager]All Shaders Loaded!");
}

VkShaderModule FileManager::CreateShaderModule(const std::vector<char>& shaderCode, VkDevice* logicalDevice)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("[File Manager] failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> FileManager::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("[File Manager] failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}