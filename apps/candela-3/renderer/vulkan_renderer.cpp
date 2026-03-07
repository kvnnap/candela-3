module;

#include "vulkan/vk_platform.h"

module candela.renderer;

// import vulkan;
// import external.glfw;

using candela::renderer::VulkanRenderer;

VulkanRenderer::VulkanRenderer()
    : window(), instance(nullptr), debugMessenger(nullptr), enableValidationLayers()
{
    // init();
    #ifdef NDEBUG
    enableValidationLayers = false;
    #else
    enableValidationLayers = true;
    #endif
}

VulkanRenderer::~VulkanRenderer()
{
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


std::vector<const char*> VulkanRenderer::getRequiredInstanceExtensions()
{
    std::uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers)
    {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    return extensions;
}

std::vector<const char*> VulkanRenderer::getRequiredLayers()
{
    // validation layers
    const std::vector<char const*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    // Get the required layers
    std::vector<char const*> requiredLayers;
    if (enableValidationLayers)
    {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    return requiredLayers;
}

void VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers) return;
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

void VulkanRenderer::init()
{
    // Query the extensions needed by GLFW
    auto requiredExtensions = getRequiredInstanceExtensions();
    
    // Check if the extensions needed by glfw are supported by Vulkan
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (auto i = 0u; i < requiredExtensions.size(); ++i)
    {
        const auto res = std::ranges::none_of(extensionProperties,
            [requiredExtension = requiredExtensions[i]](const auto& extensionProperty)
            { 
                return std::strcmp(extensionProperty.extensionName, requiredExtension) == 0; 
            });
        if (res)
            throw std::runtime_error("Required extension not supported: " + std::string(requiredExtensions[i]));
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto requiredLayers = getRequiredLayers();
    auto layerProperties = context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                    [&layerProperties](auto const &requiredLayer) {
                                                        return std::ranges::none_of(layerProperties,
                                                                                    [requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                    });
    if (unsupportedLayerIt != requiredLayers.end())
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));

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

static void handleGLFWError(int result)
{
    using namespace glfw;
    if (result != glfw_false)
        return;

    const char* description {};
    const auto code = glfwGetError(&description);
    throw std::runtime_error("vulkan_renderer: glfwInit() failed, code: " 
        + std::to_string(code) + ", description: " + std::string(description));
    
}

void VulkanRenderer::initWindow()
{
    using namespace glfw;
    constexpr unsigned WIDTH = 800;
    constexpr unsigned HEIGHT = 600;

    auto result = glfwInit();
    handleGLFWError(result);

    glfwWindowHint(glfw_client_api, glfw_no_api);
    glfwWindowHint(glfw_resizable, glfw_false);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Kevin", nullptr, nullptr);
    handleGLFWError(window == nullptr ? glfw_false : glfw_true);
}


void VulkanRenderer::renderFrame()
{
    
}

std::optional<bool> VulkanRenderer::processMessages() 
{
    using namespace glfw;

    int shouldClose {};
    while (!(shouldClose = glfwWindowShouldClose(window)))
        glfwPollEvents();
    return shouldClose;
}

void VulkanRenderer::cleanup()
{
    using namespace glfw;
    glfwDestroyWindow(window);
    glfwTerminate();
}