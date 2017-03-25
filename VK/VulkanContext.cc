#include <iostream>
#include <set>

#include "VulkanContext.hh"
#include "../Context.hh"

using namespace Animate::VK;

VulkanContext::VulkanContext(std::shared_ptr<Animate::Context> context)
{
    this->context = std::weak_ptr<Animate::Context>(context);

    this->create_instance();
    this->bind_debug_callback();
    this->create_surface();
    this->pick_physical_device();
    this->create_logical_device();

    this->context.lock()->set_vulkan(this);
}

VulkanContext::~VulkanContext()
{
}

void VulkanContext::create_instance()
{
    //Check that the required extensions are available
    if (!this->check_instance_extensions()) {
        throw std::runtime_error("Missing required vulkan extension.");
    }

    //Check for required layers
    if (!this->check_instance_layers()) {
        throw std::runtime_error("Missing required vulkan layer.");
    }

    //Get the required extensions and layers
    std::vector<const char*> extensions = this->get_required_instance_extensions();
    std::vector<const char*> layers = this->get_required_instance_layers();
    std::vector<const char*>::iterator it = extensions.begin();

    //Create a vulkan instance
    vk::ApplicationInfo app_info = vk::ApplicationInfo()
        .setPApplicationName("Animate")
        .setApplicationVersion(1)
        .setPEngineName("No Engine")
        .setEngineVersion(1)
        .setApiVersion(VK_API_VERSION_1_0);

    vk::InstanceCreateInfo create_info = vk::InstanceCreateInfo()
        .setPApplicationInfo(&app_info)
        .setEnabledExtensionCount(extensions.size())
        .setPpEnabledExtensionNames(extensions.data())
        .setEnabledLayerCount(layers.size())
        .setPpEnabledLayerNames(layers.data());

    vk::createInstance(&create_info, nullptr, &this->instance);
}

void VulkanContext::bind_debug_callback()
{
    //Set the debug callback
    vk::DebugReportCallbackCreateInfoEXT debug_create_info = vk::DebugReportCallbackCreateInfoEXT()
    .setFlags(vk::DebugReportFlagBitsEXT::eInformation | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning | vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eDebug)
    .setPfnCallback((PFN_vkDebugReportCallbackEXT)debug_callback);

    auto CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)this->instance.getProcAddr("vkCreateDebugReportCallbackEXT");
    if (CreateDebugReportCallback != nullptr) {
        VkDebugReportCallbackEXT callback;
        const VkDebugReportCallbackCreateInfoEXT tmp(debug_create_info);
        CreateDebugReportCallback(this->instance, &tmp, nullptr, &callback);
    } else {
        throw std::runtime_error("Cannot find required vkCreateDebugReportCallbackEXT function.");
    }
}

void VulkanContext::pick_physical_device()
{
    uint32_t device_count = 0;
    this->instance.enumeratePhysicalDevices(&device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("No suitable GPU device found.");
    }

    std::vector<vk::PhysicalDevice> devices(device_count);
    this->instance.enumeratePhysicalDevices(&device_count, devices.data());

    //Just pick the first device for now
    for (auto const & device : devices) {
        if (this->is_device_suitable(device)) {
            this->physical_device = device;
            break;
        }
    }

    if (!this->physical_device) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void VulkanContext::create_logical_device()
{
    QueueFamilyIndices indices = this->get_device_queue_families(this->physical_device);

    float const priority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<int> queue_families = {
        indices.graphics_family,
        indices.present_family
    };

    for (int queue_family : queue_families) {
        queue_create_infos.push_back(
            vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queue_family)
                .setQueueCount(1)
                .setPQueuePriorities(&priority)
        );
    }

    vk::PhysicalDeviceFeatures features = vk::PhysicalDeviceFeatures();

    std::vector<const char*> layers = this->get_required_instance_layers();
    std::vector<const char*> extensions = this->get_required_device_extensions();

    vk::DeviceCreateInfo device_create_info = vk::DeviceCreateInfo()
        .setPQueueCreateInfos(queue_create_infos.data())
        .setQueueCreateInfoCount(queue_create_infos.size())
        .setPEnabledFeatures(&features)
        .setEnabledLayerCount(layers.size())
        .setPpEnabledLayerNames(layers.data())
        .setEnabledExtensionCount(extensions.size())
        .setPpEnabledExtensionNames(extensions.data());

    this->physical_device.createDevice(&device_create_info, nullptr, &this->logical_device);

    this->graphics_queue = this->logical_device.getQueue(indices.graphics_family, 0);
    this->present_queue = this->logical_device.getQueue(indices.present_family, 0);
}

void VulkanContext::create_surface()
{
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(this->instance, this->context.lock()->get_window(), NULL, &surface);
    if (err) {
        throw std::runtime_error("Couldn't create window surface.");
    }
    this->context.lock()->set_surface(new vk::SurfaceKHR(surface));
}

bool VulkanContext::is_device_suitable(vk::PhysicalDevice device)
{
    QueueFamilyIndices queue_families = this->get_device_queue_families(device);
    bool extensions_suported = this->check_device_extensions(device);

    bool swap_chain_adequate = false;
    if (extensions_suported) {
        SwapChainSupportDetails swap_chain_support = get_swap_chain_support(device);
        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
    }

    //vk::PhysicalDeviceProperties properties = device.getProperties();
    vk::PhysicalDeviceFeatures features = device.getFeatures();

    return features.geometryShader && queue_families.is_complete() && swap_chain_adequate;
}

QueueFamilyIndices VulkanContext::get_device_queue_families(vk::PhysicalDevice device)
{
    uint32_t property_count = 0;
    device.getQueueFamilyProperties(&property_count, nullptr);

    std::vector<vk::QueueFamilyProperties> properties(property_count);
    device.getQueueFamilyProperties(&property_count, properties.data());

    QueueFamilyIndices indices;
    int i = 0;
    for (const auto& property : properties) {
        //Check whether our surface can be drawn to with this device
        vk::Bool32 present_supported = false;
        device.getSurfaceSupportKHR(i, *this->context.lock()->get_surface().get(), &present_supported);

        if (property.queueCount > 0) {
            if (property.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphics_family = i;
            }

            if (present_supported) {
                indices.present_family = i;
            }
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }
    return indices;
}

SwapChainSupportDetails VulkanContext::get_swap_chain_support(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;

    vk::SurfaceKHR surface = *this->context.lock()->get_surface().get();

    device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

    uint32_t format_count;
    device.getSurfaceFormatsKHR(surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        device.getSurfaceFormatsKHR(surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    device.getSurfacePresentModesKHR(surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        device.getSurfacePresentModesKHR(surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

std::vector<const char*> VulkanContext::get_required_instance_extensions() const
{
    std::vector<const char*> extensions;

    unsigned int glfw_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    for (unsigned int i=0; i<glfw_extension_count; i++) {
        extensions.push_back(glfw_extensions[i]);
    }

    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    return extensions;
}

std::vector<vk::ExtensionProperties> VulkanContext::get_avalable_instance_extensions() const
{
    uint32_t extension_count = 0;
    vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<vk::ExtensionProperties> extensions(extension_count);
    vk::enumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

    return extensions;
}

std::vector<const char*> VulkanContext::get_required_instance_layers() const
{
    return {
        "VK_LAYER_LUNARG_standard_validation"
    };
}

std::vector<vk::LayerProperties> VulkanContext::get_available_instance_layers() const
{
    uint32_t layer_count;
    vk::enumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<vk::LayerProperties> layers(layer_count);
    vk::enumerateInstanceLayerProperties(&layer_count, layers.data());

    return layers;
}

std::vector<const char*> VulkanContext::get_required_device_extensions() const
{
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

std::vector<vk::ExtensionProperties> VulkanContext::get_available_device_extensions(vk::PhysicalDevice device) const
{
    uint32_t extension_count;
    device.enumerateDeviceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<vk::ExtensionProperties> extensions(extension_count);
    device.enumerateDeviceExtensionProperties(nullptr, &extension_count, extensions.data());

    return extensions;
}

bool VulkanContext::check_instance_extensions()
{
    //Get extensions required by glfw
    std::vector<const char*> const required_extensions = this->get_required_instance_extensions();

    //Get available extensions
    std::vector<vk::ExtensionProperties> const available_extensions = this->get_avalable_instance_extensions();

    //Check we have all the required extensions
    bool found;
    for (const auto& required_extension : required_extensions) {
        found = false;
        for (const auto& available_extension : available_extensions) {
            if (strcmp(required_extension, available_extension.extensionName) == 0){
                found = true;
            }
        }
        if (!found){
            return false;
        }
    }

    return true;
}

bool VulkanContext::check_instance_layers()
{
    std::vector<const char*> const required_layers = this->get_required_instance_layers();
    std::vector<vk::LayerProperties> const available_layers = this->get_available_instance_layers();


    for (const auto& layer_name : required_layers) {
        bool found = false;

        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

bool VulkanContext::check_device_extensions(vk::PhysicalDevice device)
{
    std::vector<vk::ExtensionProperties> available_extensions = this->get_available_device_extensions(device);
    std::vector<const char *> required_extensions_cstr = this->get_required_device_extensions();

    std::set<std::string> required_extensions(required_extensions_cstr.begin(), required_extensions_cstr.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debug_callback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT object_type,
    uint64_t object,
    size_t location,
    int32_t code,
    const char* layer_prefix,
    const char* msg,
    void* user_data
) {
    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}
