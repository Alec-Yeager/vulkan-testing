#include "BasicApp.hpp"
#include <stdexcept>
#include <array>

namespace vpe
{
    BasicApp::BasicApp()
    {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    BasicApp::~BasicApp()
    {
        vkDestroyPipelineLayout(vpeDevice_.device(), pipelineLayout_, nullptr);
    }

    void BasicApp::run()
    {
        while (!vpeWindow_.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }
    }

    void BasicApp::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(vpeDevice_.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout.");
        }
    }

    void BasicApp::createPipeline()
    {
        // It's important to use the swapchain w,h because it might not match the window's lol
        auto pipelineConfig = VpePipeline::defaultPipelineConfigInfo(vpeSwapChain_.width(), vpeSwapChain_.height());
        // We'll eventually get a render pass from elsewhere.
        // It describes the structure and format of the framebuffer and its attachemnts.
        pipelineConfig.renderPass = vpeSwapChain_.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout_;
        vpePipeline_ = std::make_unique<VpePipeline>(
            vpeDevice_,
            "shaders/SimpleVertex.vert.spv",
            "shaders/SimpleFragment.frag.spv",
            pipelineConfig);
    }

    void BasicApp::createCommandBuffers()
    {
        // A bit unclear still on what exactly these really are.
        // It seems to be a set of commands that correspond roughly one to one with frambuffers.
        commandBuffers_.resize(vpeSwapChain_.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        // two types of command buffers.
        // Primary can be submitted for execution but not referenced by other command buffers.
        // Secondary can be referenced by other buffers but not submitted for execution.
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        // This is a section of memory allocated at start so that Vulkan can intelligently reuse the memory.
        allocInfo.commandPool = vpeDevice_.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

        if (vkAllocateCommandBuffers(vpeDevice_.device(), &allocInfo, commandBuffers_.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers.");
        }

        // Now we record our draw commands to the buffers.
        for (int i = 0; i < commandBuffers_.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers_[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = vpeSwapChain_.getRenderPass();
            renderPassInfo.framebuffer = vpeSwapChain_.getFrameBuffer(i);
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = vpeSwapChain_.getSwapChainExtent();

            // This is the initial value of the frame buffer attachments.
            // For us, index 0 is the color attachment, index 1 is the depth attachment.
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            // Furthest value is one, closest is 0
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            // Vk subpass contents arg says subsequent render pass commands will be directly embedded in the primary cmd buffer. No secondary.
            // Alernative is to use secondary commands rather than inline.
            // We CANNOT mix the two. Either all inline, or all secondary.
            vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vpePipeline_->bind(commandBuffers_[i]);
            vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);

            // Now we end the render pass.
            vkCmdEndRenderPass(commandBuffers_[i]);
            if (vkEndCommandBuffer(commandBuffers_[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to record command buffer.");
            }
        }
    }

    void BasicApp::drawFrame()
    {
        uint32_t imageIndex;
        // This gets the index of the fram we should render to next.
        auto result = vpeSwapChain_.acquireNextImage(&imageIndex);

        // We'll need to handle this later for resizing.
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acqure swap chain image!");
        }

        // submits the command buffer, handles cpu gpu sync
        // buffer is then executed, and the swapchain presents the associated color attachment imageview to display
        // based on the present mode given
        result = vpeSwapChain_.submitCommandBuffers(&commandBuffers_[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image.");
        }
    }
}