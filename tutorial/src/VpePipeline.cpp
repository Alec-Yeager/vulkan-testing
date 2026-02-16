#include "VpePipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace vpe
{
    VpePipeline::VpePipeline(
        VpeDevice &device,
        const fs::path &vertFilePath,
        const fs::path &fragFilepath,
        const PipelineConfigInfo &configInfo) : vpeDevice_{device}
    {
        createGraphicsPipeline(vertFilePath, fragFilepath, configInfo);
    }

    PipelineConfigInfo VpePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo configInfo{};
        return configInfo;
    }

    std::vector<char> VpePipeline::readFile(const fs::path &filepath)
    {
        std::ifstream file(filepath, std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filepath.string());
        }

        size_t fileSize = fs::file_size(filepath);
        std::vector<char> buffer(fileSize);

        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    void VpePipeline::createGraphicsPipeline(
        const fs::path &vertFilePath,
        const fs::path &fragFilepath,
        const PipelineConfigInfo &configInfo)
    {
        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilepath);
        SPDLOG_DEBUG("Vertex Code size: {}", vertCode.size());
        SPDLOG_DEBUG("Fragment Code size: {}", fragCode.size());
    }

    void VpePipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        // This had some warnings about it. Seems a c style array isn't guaranteed to work here.
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(vpeDevice_.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module.");
        }
    }
}