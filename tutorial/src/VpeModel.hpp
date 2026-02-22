#pragma once

#include "VpeDevice.hpp"
#define GLM_FORCE_RADIANS
// The default for OpenGL is -1 to 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>

namespace vpe
{
    class VpeModel
    {
    public:
        struct Vertex
        {
            glm::vec2 position;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        VpeModel(VpeDevice &device, const std::vector<Vertex> &vertices);
        ~VpeModel();

        VpeModel(const VpeModel &) = delete;
        VpeModel &operator=(const VpeModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);

        VpeDevice &vpeDevice_;
        // Interestingly, the buffer and the memory are seperate objects.
        VkBuffer vertexBuffer_;
        VkDeviceMemory vertexBufferMemory_;
        uint32_t vertexCount_;
    };

} // namespace vpe
