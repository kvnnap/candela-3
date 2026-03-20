module candela.renderer;

import core.util;

using candela::renderer::VulkanDevice;
using candela::renderer::VulkanInstance;
using candela::renderer::VulkanSwapchain;

VulkanDevice::VulkanDevice(const VulkanInstance& instance, const VulkanSwapchain& swapchain)
    : instance(instance), swapchain(swapchain), physicalDevice(nullptr), 
      device(nullptr), graphicsQueue(nullptr), graphicsQueueFamilyIndex()
{}

void VulkanDevice::init()
{
    pickPhysicalDevice();
    createLogicalDevice();
}

void VulkanDevice::wait() const
{
    if (graphicsQueue != nullptr)
        graphicsQueue.waitIdle();
}

const vk::raii::PhysicalDevice& VulkanDevice::getPhysicalDevice() const { return physicalDevice; }
const vk::raii::Device& VulkanDevice::getDevice() const { return device; }
const vk::raii::Queue& VulkanDevice::getGraphicsQueue() const { return graphicsQueue; }
std::uint32_t VulkanDevice::getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex; }

void VulkanDevice::pickPhysicalDevice()
{
    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};
    const auto& surface = swapchain.getSurface();

    auto physicalDevices = instance.getInstance().enumeratePhysicalDevices();
    if (physicalDevices.empty())
        throw std::runtime_error("Failed to find GPUs with Vulkan Support!");
    for (auto &phyDevice : physicalDevices)
    {
        
        auto queueFamilies = phyDevice.getQueueFamilyProperties();
        bool supportGraphics{};
        for(auto i = 0u; i < queueFamilies.size(); ++i)
        {
            const auto& qfp = queueFamilies[i];
            if (static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics) && phyDevice.getSurfaceSupportKHR(i, surface))
            {
                supportGraphics = true;
                break;
            }
        }

        if (!supportGraphics)
            continue;

        auto availableDevExtensions = phyDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = core::util::AllElemsInSet(availableDevExtensions, requiredDeviceExtension, extComparator);
        if (!supportsAllRequiredExtensions)
            continue;

        auto features = phyDevice.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        const auto& pdv13features = features.get<vk::PhysicalDeviceVulkan13Features>();
        bool supportsRequiredFeatures = pdv13features.dynamicRendering && pdv13features.synchronization2 &&
                                        features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                        features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
                                
        if (!supportsRequiredFeatures)
            continue;

        physicalDevice = phyDevice;
        auto props = phyDevice.getProperties();
        auto apiVersion = std::format("{}.{}.{}", vk::apiVersionMajor(props.apiVersion), vk::apiVersionMinor(props.apiVersion), vk::apiVersionPatch(props.apiVersion));
        std::cout << std::format("Device name: {}, Api Version: {}", props.deviceName.data(), apiVersion) << std::endl;
        break;
    }

    if (physicalDevice == nullptr)
        throw std::runtime_error("No compatible device found");
}

void VulkanDevice::createLogicalDevice()
{
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    const auto& surface = swapchain.getSurface();

    std::uint32_t queueIndex;
    for(queueIndex = 0u; queueIndex < queueFamilyProperties.size(); ++queueIndex)
    {
        const auto& qfp = queueFamilyProperties[queueIndex];
        if (static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(queueIndex, surface))
            break;
    }
    if (queueIndex == queueFamilyProperties.size())
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

    const float prio = 0.5f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo { 
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &prio
    };

    // these are the features we queried for earlier when picking the device.
    // It's a chain PhysicalDeviceFeatures2 -> PhysicalDeviceVulkan13Features -> PhysicalDeviceExtendedDynamicStateFeaturesEXT
    // Driver enables featueres in all these structs
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
        {},                               // vk::PhysicalDeviceFeatures2 (empty for now)
        {.shaderDrawParameters = true },      // Enable shaderDrawParameters rendering from Vulkan 1.1
        {.synchronization2 = true, .dynamicRendering = true },      // Enable dynamic rendering and sync from Vulkan 1.3
        {.extendedDynamicState = true }   // Enable extended dynamic state from the extension
    };

    // We didnt check for this earlier? not sure
    std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };

    vk::DeviceCreateInfo deviceCreateInfo {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
    graphicsQueueFamilyIndex = queueIndex;
}