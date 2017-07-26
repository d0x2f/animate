#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>

namespace Animate::VK
{
    class Context;
    class Buffer;

    class Texture
    {
        public:
            Texture(std::weak_ptr<Context> context, std::string resource_id);
            ~Texture();

        private:
            std::weak_ptr<Context> context;
            vk::Device logical_device;
            vk::Image image;
            vk::DeviceMemory memory;

            void create_image(uint32_t width, uint32_t height);
            void transition_image_layout(vk::ImageLayout old_layout, vk::ImageLayout new_layout);
            void copy_buffer_to_image(VK::Buffer &staging_buffer, uint32_t width, uint32_t height);
    };
}
