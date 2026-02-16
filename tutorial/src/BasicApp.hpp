#pragma once

#include "VpeWindow.hpp"
#include "VpePipeline.hpp"
#include "VpeDevice.hpp"

namespace vpe
{
    class BasicApp
    {

    public:
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;

        void run();

    private:
        VpeWindow vpeWindow_{WIDTH, HEIGHT, "FIRST WINDOW!"};
        VpeDevice vpeDevice_{vpeWindow_};
        VpePipeline vpePipeline_{vpeDevice_,
                                 "shaders/SimpleVertex.vert.spv",
                                 "shaders/SimpleFragment.frag.spv",
                                 VpePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
} // namespace vpe
