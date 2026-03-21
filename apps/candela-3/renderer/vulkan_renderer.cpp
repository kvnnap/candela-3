module candela.renderer;

import core.util;

using candela::renderer::VulkanRenderer;
using candela::renderer::VulkanInstance;

template<typename T>
static void setDebugName(const vk::raii::Device& device, T& obj, const std::string& name)
{
    vk::DebugUtilsObjectNameInfoEXT nameInfo {
        .objectType = T::objectType,
        .objectHandle = reinterpret_cast<std::uintptr_t>(static_cast<T::CType>(*obj)),
        .pObjectName = name.c_str()
    };
    device.setDebugUtilsObjectNameEXT(nameInfo);
}

VulkanRenderer::VulkanRenderer()
    : commandPool(nullptr),
      commandBuffer(nullptr),
      frameIndex()
{
    // init();
}

VulkanRenderer::~VulkanRenderer()
{
    if (device)
        device->wait();
}

void VulkanRenderer::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ 
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
        .queueFamilyIndex = device->getGraphicsQueueFamilyIndex() 
    };
    commandPool = vk::raii::CommandPool(device->getDevice(), poolInfo);
}

void VulkanRenderer::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo { 
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };
    commandBuffers = vk::raii::CommandBuffers(device->getDevice(), allocInfo);
    if constexpr(enableValidationLayers)
    {
        for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i)
            setDebugName(device->getDevice(), commandBuffers[i], std::format("commandBuffers[{}]", i));
    }
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
        .image = swapchain->getImage(imageIndex),
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
    commandBuffer->pipelineBarrier2(dependencyInfo);
}

void VulkanRenderer::recordCommandBuffer(std::uint32_t imageIndex)
{
    commandBuffer->begin({});

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
        .imageView = swapchain->getImageView(imageIndex),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    // Set up the rendering info
    const auto& extent = swapchain->getExtent();
    vk::RenderingInfo renderingInfo = {
        .renderArea = { .offset = { 0, 0 }, .extent = extent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    // Begin rendering
    commandBuffer->beginRendering(renderingInfo);

    // // Rendering commands will go here
    commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());

    commandBuffer->setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));
    commandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

    commandBuffer->draw(3, 1, 0, 0);

    // End rendering
    commandBuffer->endRendering();

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

    commandBuffer->end();
}

void VulkanRenderer::createSyncObjects()
{
    const auto& dev = device->getDevice();
    for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        presentCompleteSemaphores.emplace_back(dev, vk::SemaphoreCreateInfo());
        drawFences.emplace_back(dev, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
    }

    for (auto i = 0u; i < swapchain->getSize(); ++i)
        renderFinishedSemaphores.emplace_back(dev, vk::SemaphoreCreateInfo());

    if constexpr(enableValidationLayers)
    {
        for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            setDebugName(dev, presentCompleteSemaphores[i], std::format("presentCompleteSemaphores[{}]", i));
            setDebugName(dev, drawFences[i], std::format("drawFences[{}]", i));
        }
        for (auto i = 0u; i < renderFinishedSemaphores.size(); ++i)
            setDebugName(dev, renderFinishedSemaphores[i], std::format("renderFinishedSemaphores[{}]", i));
    }
}

void VulkanRenderer::init()
{
    window = std::make_unique<core::window::GLFWWindow>("Candela 3", 800, 600);
    instance = std::make_unique<VulkanInstance>(*window);
    instance->init();

    swapchain = std::make_unique<VulkanSwapchain>(*window, *instance);
    swapchain->initSurface();

    device = std::make_unique<VulkanDevice>(*instance, *swapchain);
    device->init();

    swapchain->setDevice(*device);
    swapchain->initSwapchain();

    pipeline = std::make_unique<VulkanPipeline>(*device, *swapchain);
    pipeline->init();
    
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

void VulkanRenderer::renderFrame()
{
    const auto frameMod = getFrameModulo();
    auto &drawFence = drawFences[frameMod];
    auto &presentCompleteSemaphore = presentCompleteSemaphores[frameMod];
    commandBuffer = &commandBuffers[frameMod];

    // graphicsQueue.waitIdle();
    auto& dev = device->getDevice();
    dev.waitForFences(*drawFence, vk::True, std::numeric_limits<std::uint64_t>().max());
    dev.resetFences(*drawFence);
    
    auto [result, imageIndex] = swapchain->getSwapchain().acquireNextImage(std::numeric_limits<std::uint64_t>().max(), *presentCompleteSemaphore, nullptr);
    auto &renderFinishedSemaphore = renderFinishedSemaphores[imageIndex];
    // std::cout << imageIndex << ",";
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &**commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphore
    };

    device->getGraphicsQueue().submit(submitInfo, *drawFence);
    
    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapchain->getSwapchain(),
        .pImageIndices = &imageIndex
    };

    result = device->getGraphicsQueue().presentKHR(presentInfoKHR);
    ++frameIndex;
}

bool VulkanRenderer::processMessages() 
{
    return window->processMessages();
}

std::uint32_t VulkanRenderer::getFrameModulo() const
{
    return frameIndex % MAX_FRAMES_IN_FLIGHT;
}
