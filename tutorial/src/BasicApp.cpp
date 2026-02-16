#include "BasicApp.hpp"

namespace vpe
{
    void BasicApp::run()
    {
        while (!vpeWindow_.shouldClose())
        {
            glfwPollEvents();
        }
    }
}