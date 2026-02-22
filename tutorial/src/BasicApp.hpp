#pragma once

#include "VpeWindow.hpp"
#include "VpePipeline.hpp"
#include "VpeDevice.hpp"
#include "VpeSwapChain.hpp"
#include <memory>
#include <vector>

namespace vpe
{
    class BasicApp
    {

    public:
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;

        BasicApp();
        ~BasicApp();

        BasicApp(const BasicApp &) = delete;
        BasicApp &operator=(const BasicApp &) = delete;

        void run();

    private:
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        VpeWindow vpeWindow_{WIDTH, HEIGHT, "FIRST WINDOW!"};
        VpeDevice vpeDevice_{vpeWindow_};
        VpeSwapChain vpeSwapChain_{vpeDevice_, vpeWindow_.getExtent()};
        std::unique_ptr<VpePipeline> vpePipeline_;
        VkPipelineLayout pipelineLayout_;
        std::vector<VkCommandBuffer> commandBuffers_;
    };
} // namespace vpe
