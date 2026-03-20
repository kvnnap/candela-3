module;

// Used for vkDebugCallback
#include "vulkan/vk_platform.h"

module candela.renderer;

import core.util;

using candela::renderer::VulkanInstance;

static auto layerComparator = [](const vk::LayerProperties& l, const char* r){ return std::string_view(l.layerName) == r; };

static void throwIfNotSupported(const auto& availableExtensions, const auto& reqExtensions, auto &comparator, const std::string& throwMessagePrefix)
{
    auto requiredThatFailed = core::util::GetElemsNotInSet(availableExtensions, reqExtensions, comparator);
    if (requiredThatFailed.empty())
        return;
    auto reqJoined = requiredThatFailed 
        | std::views::transform([](auto x){ return std::string_view(*x); })
        | std::views::join_with(std::string(", "))
        | std::ranges::to<std::string>();

    throw std::runtime_error(throwMessagePrefix + ": " + reqJoined);
}

std::vector<const char*> VulkanInstance::getRequiredInstanceExtensions()
{
    auto extensions = window.getRequiredVulkanExtensions();

    if constexpr(enableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

std::vector<const char*> VulkanInstance::getRequiredLayers()
{
    // validation layers
    const std::vector<char const*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    // Get the required layers
    std::vector<char const*> requiredLayers;
    if constexpr(enableValidationLayers)
    {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    return requiredLayers;
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT              type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                                                      void *                                         pUserData)
{
  std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
  if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
  {
      // Message is important enough to show
  }

  return vk::False;
}

VulkanInstance::VulkanInstance(const core::window::IVulkanWindow& window)
    : window(window), instance(nullptr), debugMessenger(nullptr)
{}

void VulkanInstance::setupDebugMessenger()
{
    if constexpr(enableValidationLayers) 
    {
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,
                                                                            .messageType     = messageTypeFlags,
                                                                            .pfnUserCallback = &vkDebugCallback};
        debugMessenger = instance.createDebugUtilsMessengerEXT( debugUtilsMessengerCreateInfoEXT );
    }
}

vk::raii::Instance& VulkanInstance::getInstance()
{
    return instance;
}

void VulkanInstance::init()
{
        // Query the extensions needed by GLFW
    auto requiredExtensions = getRequiredInstanceExtensions();
    
    // Check if the extensions needed by glfw are supported by Vulkan
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    throwIfNotSupported(extensionProperties, requiredExtensions, extComparator, "Required extension(s) not supported");

    // Check if the required layers are supported by the Vulkan implementation.
    auto requiredLayers = getRequiredLayers();
    auto layerProperties = context.enumerateInstanceLayerProperties();
    throwIfNotSupported(layerProperties, requiredLayers, layerComparator, "Required layer(s) not supported");

    vk::ApplicationInfo appInfo{
        .pApplicationName = "Candela",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };
    
    try 
    {
        instance = vk::raii::Instance(context, createInfo);
    } catch (const vk::SystemError& err)
    {
        std::cerr << "Vulkan error: " << err.what() << std::endl;
        throw;
    }

    setupDebugMessenger();
}