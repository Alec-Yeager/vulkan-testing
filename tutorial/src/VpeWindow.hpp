#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vpe
{

    class VpeWindow
    {
    public:
        VpeWindow(int w, int h, std::string name);
        ~VpeWindow();

        // Disallow copy constructor. We're using a raw pointer, don't want a dangling pointer from a copy getting deleted.
        VpeWindow(const VpeWindow &) = delete;
        VpeWindow &operator=(const VpeWindow &) = delete;

        bool shouldClose();
        VkExtent2D getExtent()
        {
            return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)};
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        void initWindow();

        const int width_;
        const int height_;

        std::string windowName_;
        GLFWwindow *window_;
    };
}