#pragma once

#include "VpeDevice.hpp"

#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace vpe
{
    struct PipelineConfigInfo
    {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class VpePipeline
    {
    public:
        VpePipeline(
            VpeDevice &device,
            const fs::path &vertFilePath,
            const fs::path &fragFilepath,
            const PipelineConfigInfo &configInfo);
        ~VpePipeline();

        // Remove the copy constructors again.
        VpePipeline(const VpePipeline &) = delete;
        void operator=(const VpePipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    private:
        static std::vector<char> readFile(const fs::path &filepath);
        void createGraphicsPipeline(
            const fs::path &vertFilePath,
            const fs::path &fragFilepath,
            const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        // This is a reference, which could be unsafe.
        // But the whole pipeline is useless without it, so we assume it won't get nuked before this class.
        VpeDevice &vpeDevice_;
        VkPipeline graphicsPipeline_;
        VkShaderModule vertShaderModule_;
        VkShaderModule fragShaderModule_;
    };
} // namespace vpe
