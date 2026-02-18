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

    VpePipeline::~VpePipeline()
    {
        vkDestroyShaderModule(vpeDevice_.device(), vertShaderModule_, nullptr);
        vkDestroyShaderModule(vpeDevice_.device(), fragShaderModule_, nullptr);
        vkDestroyPipeline(vpeDevice_.device(), graphicsPipeline_, nullptr);
    }

    PipelineConfigInfo VpePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo configInfo{};

        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        // This means input geometry is a list of triangles, not like lines or something.
        // Every three verts are grouped into triangles.
        // Could use STRIP instead of LIST, which would link each vert to the previous two.
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Defines the viewport transform.
        // If the w,h are set weird, the vertex positions will be squished accordingly.
        configInfo.viewport.x = 0.0f;
        configInfo.viewport.y = 0.0f;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0f;
        configInfo.viewport.maxDepth = 1.0f;

        // The scissor cuts off the view within the given box.
        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // Not forcing clamp on 0,1 depth values. If <0, behind camera. If >1, too far to see.
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        // Obviously we want to rasterize.
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        // Fill polygons, lineWidth is obv
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        // Optionally discard tris based on facing (winding order)
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        // Depth bias is used to alter depth vals by adding a constant or mult by a slope (later tool)
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        // Multisampling is for when a fragment leans over multiple pixels. We need this.
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;
        configInfo.multisampleInfo.pSampleMask = nullptr;
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

        // If we have overlapping triangles, this is how we control our blending.
        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

        // Stores depth info of the top fragment.
        // We only update if a new fragment is closer than the current in that location
        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};
        configInfo.depthStencilInfo.back = {};

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

        assert(
            configInfo.pipelineLayout != VK_NULL_HANDLE &&
            "configInfo needs a pipelineLayout.");
        assert(
            configInfo.renderPass != VK_NULL_HANDLE &&
            "configInfo needs a renderPass.");

        auto vertCode = readFile(vertFilePath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule_);
        createShaderModule(fragCode, &fragShaderModule_);

        VkPipelineShaderStageCreateInfo shaderStages[2];

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule_;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule_;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        // We set the viewportInfo to have the viewport and scissor.
        VkPipelineViewportStateCreateInfo viewportInfo;
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &configInfo.viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &configInfo.scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(vpeDevice_.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }
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