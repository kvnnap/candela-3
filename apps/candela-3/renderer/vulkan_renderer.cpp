module;

// Used for vkDebugCallback
#include "vulkan/vk_platform.h"

module candela.renderer;

// import vulkan;
// import external.glfw;

using candela::renderer::VulkanRenderer;

VulkanRenderer::VulkanRenderer()
    : window(), 
      instance(nullptr),
      debugMessenger(nullptr),
      physicalDevice(nullptr),
      device(nullptr),
      graphicsQueue(nullptr), graphicsQueueFamilyIndex(),
      surface(nullptr),
      swapChain(nullptr),
      swapChainExtent(),
      swapChainSurfaceFormat(),
      pipelineLayout(nullptr),
      graphicsPipeline(nullptr),
      commandPool(nullptr), commandBuffer(nullptr),
      presentCompleteSemaphore(nullptr), drawFence(nullptr)
{
    // init();
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
    if constexpr(enableValidationLayers)
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
    if constexpr(enableValidationLayers)
    {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }

    return requiredLayers;
}

void VulkanRenderer::setupDebugMessenger()
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

void VulkanRenderer::pickPhysicalDevice()
{
    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    auto physicalDevices = instance.enumeratePhysicalDevices();
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
        bool supportsAllRequiredExtensions =
            std::ranges::all_of( requiredDeviceExtension,
            [&availableDevExtensions]( auto const & requiredDeviceExtension )
            {
                return std::ranges::any_of( availableDevExtensions,
                                            [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                            { return std::string_view(availableDeviceExtension.extensionName) == requiredDeviceExtension; } );
            } 
            );

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

void VulkanRenderer::createLogicalDevice()
{
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

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

    vk::PhysicalDeviceFeatures deviceFeatures;

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
        .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
        .ppEnabledExtensionNames = requiredDeviceExtension.data()
    };

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
    graphicsQueueFamilyIndex = queueIndex;
}

void VulkanRenderer::createSurface()
{
    vk::raii::SurfaceKHR::CType _surface{nullptr};
    if (glfw::glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0)
        throw std::runtime_error("failed to create window surface!");
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void VulkanRenderer::createSwapChain()
{
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    auto availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
    auto availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto &format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
    
    swapChainSurfaceFormat = *formatIt;
    auto presentMode = std::ranges::any_of(availablePresentModes, [](const auto& value){ return value == vk::PresentModeKHR::eMailbox; }) ?
                        vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;

    // Get window buffer size in pixels (there are other logical units)
    auto &extent = swapChainExtent;
    extent = surfaceCapabilities.currentExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<std::uint32_t>::max())
    {
        int width, height;
        glfw::glfwGetFramebufferSize(window, &width, &height);
        extent.width = std::clamp<uint32_t>(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp<uint32_t>(width, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }

    // To reduce any waiting, use +1
    auto minImageCount = surfaceCapabilities.minImageCount < surfaceCapabilities.maxImageCount ? surfaceCapabilities.minImageCount + 1 : surfaceCapabilities.minImageCount;

    // Create swap chain
    vk::SwapchainCreateInfoKHR swapChainCreateInfo {
        .surface          = *surface,
        .minImageCount    = minImageCount,
        .imageFormat      = swapChainSurfaceFormat.format,
        .imageColorSpace  = swapChainSurfaceFormat.colorSpace,
        .imageExtent      = extent,
        .imageArrayLayers = 1, // Always 1 unless sterescope 3d app
        .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform     = surfaceCapabilities.currentTransform,
        .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode      = presentMode,
        .clipped          = true,
        .oldSwapchain = nullptr
    };

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
}

void VulkanRenderer::createImageViews()
{
    // assert(swapChainImageViews.empty());
    vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType         = vk::ImageViewType::e2D,
                                                 .format           = swapChainSurfaceFormat.format,
                                                 .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
    imageViewCreateInfo.components = {
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity,
        vk::ComponentSwizzle::eIdentity
    };
    imageViewCreateInfo.subresourceRange = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1
    };

    for (auto &image : swapChainImages)
    {
        imageViewCreateInfo.image = image;
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
}

static std::vector<std::byte> loadBlob(const std::string& path)
{
    std::vector<std::byte> buffer(std::filesystem::file_size(path));
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("failed to open file: " + path);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), buffer.size()))
        throw std::runtime_error("failed to read file: " + path);
    return buffer;
}

vk::raii::ShaderModule VulkanRenderer::createShaderModule(const std::vector<std::byte>& code)
{
    vk::ShaderModuleCreateInfo createInfo{ 
        .codeSize = code.size() * sizeof(std::byte), 
        .pCode = reinterpret_cast<const uint32_t*>(code.data()) 
    };
    return { device, createInfo };
}

void VulkanRenderer::createGraphicsPipeline()
{
    // Load shader
    auto code = loadBlob("shaders/shaders.spv");
    auto shaderModule = createShaderModule(code);
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo { 
        .stage = vk::ShaderStageFlagBits::eVertex, 
        .module = shaderModule,
        .pName = "vertMain",
        .pSpecializationInfo = nullptr // For specifying constants at pipeline build time
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{ 
        .stage = vk::ShaderStageFlagBits::eFragment, 
        .module = shaderModule, 
        .pName = "fragMain" 
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{  .topology = vk::PrimitiveTopology::eTriangleList };
    // vk::Viewport { 0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f };
    // vk::Rect2D { vk::Offset2D{ 0, 0 }, swapChainExtent };

    std::vector dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{ .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data() };

    // vk::PipelineViewportStateCreateInfo viewportState({}, 1, {}, 1);
    vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };

    vk::PipelineRasterizationStateCreateInfo rasterizer{  
        .depthClampEnable = vk::False, 
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill, .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise, .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f, .lineWidth = 1.0f 
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable    = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colorBlending {
        .logicOpEnable = vk::False, 
        .logicOp =  vk::LogicOp::eCopy, 
        .attachmentCount = 1, 
        .pAttachments =  &colorBlendAttachment 
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{.rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False};

    // Uniforms
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{  .setLayoutCount = 0, .pushConstantRangeCount = 0 };
    pipelineLayout = vk::raii::PipelineLayout( device, pipelineLayoutInfo );

    // Dynamic rendering
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format };

    vk::GraphicsPipelineCreateInfo pipelineInfo{ 
        .pNext = &pipelineRenderingCreateInfo, // This structure specifies the formats of the attachments that will be used during rendering
        .stageCount = 2, .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState, .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling, .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState, 
        .layout = pipelineLayout, // The fixed layout
        .renderPass = nullptr // null because we're using dynamic rendering
    };

    graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineInfo);
}

void VulkanRenderer::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ 
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
        .queueFamilyIndex = graphicsQueueFamilyIndex 
    };
    commandPool = vk::raii::CommandPool(device, poolInfo);
}

void VulkanRenderer::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo{ 
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1 
    };
    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}


void VulkanRenderer::transitionImageLayout(std::uint32_t imageIndex, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask)
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
        .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
        .image = swapChainImages[imageIndex],
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

void VulkanRenderer::recordCommandBuffer(std::uint32_t imageIndex)
{
    commandBuffer.begin({});

    // Transition the image layout for rendering
    transitionImageLayout(
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );

    // Set up the color attachment
    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    // Set up the rendering info
    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = { 0, 0 }, .extent = swapChainExtent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    // Begin rendering
    commandBuffer.beginRendering(renderingInfo);

    // // Rendering commands will go here
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

    commandBuffer.draw(3, 1, 0, 0);

    // End rendering
    commandBuffer.endRendering();

    // Transition the image layout for presentation
    transitionImageLayout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    commandBuffer.end();
}

template<typename T>
void setDebugName(vk::raii::Device& device, T& obj, const std::string& name)
{
    vk::DebugUtilsObjectNameInfoEXT nameInfo {
        .objectType = T::objectType,
        .objectHandle = reinterpret_cast<std::uintptr_t>(static_cast<T::CType>(*obj)),
        .pObjectName = name.c_str()
    };
    device.setDebugUtilsObjectNameEXT(nameInfo);
}

void VulkanRenderer::createSyncObjects()
{
    presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    // renderFinishedSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    drawFence = vk::raii::Fence(device, {.flags = vk::FenceCreateFlagBits::eSignaled});
    for (auto i = 0u; i < swapChainImages.size(); ++i)
        renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());

    if constexpr(enableValidationLayers)
    {
        setDebugName(device, presentCompleteSemaphore, "presentCompleteSemaphore");
        for (auto i = 0u; i < renderFinishedSemaphores.size(); ++i)
            setDebugName(device, renderFinishedSemaphores[i], std::format("renderFinishedSemaphores[{}]", i));
        setDebugName(device, drawFence, "drawFence");
    }
}

void VulkanRenderer::init()
{
    // Query the extensions needed by GLFW
    auto requiredExtensions = getRequiredInstanceExtensions();
    
    // Check if the extensions needed by glfw are supported by Vulkan
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (std::string_view required : requiredExtensions)
    {
        if (!std::ranges::any_of(extensionProperties,
            [&](auto& e){ return e.extensionName == required; }))
        {
            throw std::runtime_error(
                std::format("Required extension not supported: {}", required));
        }
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto requiredLayers = getRequiredLayers();
    auto layerProperties = context.enumerateInstanceLayerProperties();
    for (std::string_view required : requiredLayers)
    {
        if (!std::ranges::any_of(layerProperties,
            [&](const auto& layer) { return layer.layerName == required; }))
        {
            throw std::runtime_error(
                std::format("Required layer not supported: {}", required));
        }
    }

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
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
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

    window = glfwCreateWindow(WIDTH, HEIGHT, "Candela 3", nullptr, nullptr);
    handleGLFWError(window == nullptr ? glfw_false : glfw_true);
}

void VulkanRenderer::renderFrame()
{
    // graphicsQueue.waitIdle();
    device.waitForFences(*drawFence, vk::True, std::numeric_limits<std::uint64_t>().max());
    device.resetFences(*drawFence);
    
    auto [result, imageIndex] = swapChain.acquireNextImage(std::numeric_limits<std::uint64_t>().max(), *presentCompleteSemaphore, nullptr);
    auto &renderFinishedSemaphore = renderFinishedSemaphores[imageIndex];
    // std::cout << imageIndex << ",";
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphore
    };

    graphicsQueue.submit(submitInfo, *drawFence);
    
    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapChain,
        .pImageIndices = &imageIndex
    };

    result = graphicsQueue.presentKHR(presentInfoKHR);
}

bool VulkanRenderer::processMessages() 
{
    using namespace glfw;
    glfwPollEvents();
    return glfwWindowShouldClose(window);
}

void VulkanRenderer::cleanup()
{
    graphicsQueue.waitIdle();

    using namespace glfw;
    glfwDestroyWindow(window);
    glfwTerminate();
}