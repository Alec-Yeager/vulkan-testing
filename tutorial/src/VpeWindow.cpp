#include "VpeWindow.hpp"

#include <stdexcept>

namespace vpe
{
    VpeWindow::VpeWindow(int w, int h, std::string name) : width_{w}, height_{h}, windowName_{name}
    {
        initWindow();
    }

    VpeWindow::~VpeWindow()
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }

    bool VpeWindow::shouldClose()
    {
        return glfwWindowShouldClose(window_);
    }

    void VpeWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void VpeWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(width_, height_, windowName_.c_str(), nullptr, nullptr);
    }
}