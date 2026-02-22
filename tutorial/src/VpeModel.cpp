#include "VpeModel.hpp"

#include <cassert>
#include <cstring>
namespace vpe
{
    VpeModel::VpeModel(VpeDevice &device, const std::vector<Vertex> &vertices) : vpeDevice_{device}
    {
        createVertexBuffers(vertices);
    }

    VpeModel::~VpeModel()
    {
        vkDestroyBuffer(vpeDevice_.device(), vertexBuffer_, nullptr);
        vkFreeMemory(vpeDevice_.device(), vertexBufferMemory_, nullptr);
    }

    void VpeModel::bind(VkCommandBuffer commandBuffer)
    {
        // we make an array of buffers (size 1 right now)
        VkBuffer buffers[] = {vertexBuffer_};
        // These are the offsets for those
        VkDeviceSize offsets[] = {0};
        // Then we bind the first one at 0, and only one binding.
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    void VpeModel::draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, vertexCount_, 1, 0, 0);
    }

    void VpeModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount_ = static_cast<uint32_t>(vertices.size());
        assert(vertexCount_ >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount_;
        // Now we call the create buffer function in the device class.
        // It takes a buffer size, usage and props
        // Returns the buffer and its memory.

        // the usage vertex bit says this is used to hold vertex data
        // The memory properties visible and coherent are CPU (host) can see and write to them
        // The coherent bit keeps things consistent? If not there we need to VK flush the buffer, idgaf
        vpeDevice_.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertexBuffer_,
            vertexBufferMemory_);

        void *data;
        vkMapMemory(vpeDevice_.device(), vertexBufferMemory_, 0, bufferSize, 0, &data);
        // this data pointer now has our correspondence with the gpu buffer memory

        // We now copy the data from our vertices into the HOST mapped memory region.
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        // Because it's host coeherent, the memory is auto flushed to its GPU (device) counterpart.
        // Otherwise we would need to call Flush()

        // Now we don't care about it anymore?
        vkUnmapMemory(vpeDevice_.device(), vertexBufferMemory_);
    }
}