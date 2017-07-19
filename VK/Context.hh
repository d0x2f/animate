#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <set>

namespace Animate
{
    class AppContext;

    namespace Object::Property
    {
        class Drawable;
    }

    typedef Animate::Object::Property::Drawable Drawable;

    namespace VK
    {
        class Shader;

        struct BufferInfo {
            vk::Buffer ident;
            vk::DeviceMemory memory;

            BufferInfo(vk::Buffer i = vk::Buffer(), vk::DeviceMemory m = vk::DeviceMemory()) : ident(i), memory(m) {}
        };
        
        struct QueueFamilyIndices {
            int graphics_family = -1;
            int present_family = -1;

            bool is_complete() {
                return this->graphics_family >= 0 && this->present_family >= 0;
            }
        };

        struct SwapChainSupportDetails {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;
        };

        struct DrawableComparator {
            bool operator() (
                const Drawable *lhs, 
                const Drawable *rhs
            ) const {
                return lhs < rhs;
            }
        };

        class Context
        {
            public:
                Context(std::shared_ptr<Animate::AppContext> context);
            
                vk::Instance instance;
                vk::PhysicalDevice physical_device;
                vk::Device logical_device;
                vk::Queue   graphics_queue,
                            present_queue;
                vk::SwapchainKHR swap_chain;
                std::vector<vk::Image> swap_chain_images;
                vk::Format swap_chain_image_format;
                vk::Extent2D swap_chain_extent;
                std::vector<vk::ImageView> swap_chain_image_views;
                vk::PipelineLayout pipeline_layout;
                vk::RenderPass render_pass;
                vk::PipelineCache pipeline_cache;
                vk::Pipeline pipeline;
                std::vector<vk::Framebuffer> swap_chain_framebuffers;
                vk::CommandPool command_pool;
                std::vector<vk::CommandBuffer> command_buffers;
                vk::Semaphore image_available_semaphore,
                              render_finished_semaphore;

                std::vector<vk::ShaderModule> shader_modules;
                std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;

                std::multiset<Drawable *, DrawableComparator> scene;

                void add_to_scene(Drawable *drawable);
                void render_scene();
                void add_shader_stage(vk::ShaderStageFlagBits type, std::string resource_id);

                void recreate_swap_chain();

                uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

            private:
                std::weak_ptr<Animate::AppContext> context;
                bool is_complete = false;

                void cleanup_swap_chain_dependancies();

                void create_instance();
                void bind_debug_callback();
                void pick_physical_device();
                void create_logical_device();
                void create_surface();
                void create_swap_chain();
                void create_image_views();
                void create_render_pass();
                void create_pipeline();
                void create_framebuffers();
                void create_command_pool();
                void create_command_buffers();
                void create_semaphores();

                bool is_device_suitable(vk::PhysicalDevice const & device);

                QueueFamilyIndices get_device_queue_families(vk::PhysicalDevice const & device);
                SwapChainSupportDetails get_swap_chain_support(vk::PhysicalDevice const & device);

                std::vector<const char*> get_required_instance_extensions() const;
                std::vector<vk::ExtensionProperties> get_avalable_instance_extensions() const;
                std::vector<const char*> get_required_instance_layers() const;
                std::vector<vk::LayerProperties> get_available_instance_layers() const;

                std::vector<const char*> get_required_device_extensions() const;
                std::vector<vk::ExtensionProperties> get_available_device_extensions(vk::PhysicalDevice const & device) const;

                bool check_instance_extensions();
                bool check_instance_layers();
                bool check_device_extensions(vk::PhysicalDevice const & device);

                vk::SurfaceFormatKHR choose_swap_surface_format(std::vector<vk::SurfaceFormatKHR> const & available_formats) const;
                vk::PresentModeKHR choose_swap_present_mode(std::vector<vk::PresentModeKHR> const & available_present_modes) const;
                vk::Extent2D choose_swap_extent(vk::SurfaceCapabilitiesKHR const & capabilities) const;

                static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                    VkDebugReportFlagsEXT flags,
                    VkDebugReportObjectTypeEXT object_type,
                    uint64_t object,
                    size_t location,
                    int32_t code,
                    const char* layer_prefix,
                    const char* msg,
                    void* user_data
                );
        };

    }
}
