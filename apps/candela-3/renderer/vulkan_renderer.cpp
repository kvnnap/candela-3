module candela.renderer;

import core.util;
import glm;

using candela::renderer::VulkanRenderer;
using candela::renderer::VulkanInstance;

// temporary data
struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

struct UniformBufferObject 
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

static const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

static const std::vector<std::uint16_t> indices = {
    0, 1, 2
};

VulkanRenderer::VulkanRenderer()
    : frameNumber(), swapchainUnavailble(), 
      vertexBuffer(nullptr), vertexBufferMemory(nullptr),
      indexBuffer(nullptr), indexBufferMemory(nullptr),
      totalTimeMs()
{
    // init();
}

VulkanRenderer::~VulkanRenderer()
{
    if (device)
        device->wait();
}

void VulkanRenderer::recordCommandBuffer(std::uint32_t imageIndex, std::uint32_t frameIndex)
{
    auto &commandBuffer = command->getCommandBuffer(frameIndex);
    commandBuffer.begin({});

    // Transition the image layout for rendering
    transitionImageLayout(
        swapchain->getImage(imageIndex),
        command->getCommandBuffer(frameIndex),
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
    commandBuffer.beginRendering(renderingInfo);

    // // Rendering commands will go here
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getGraphicsPipeline());
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->getPipelineLayout(), 0, *pipeline->getDescriptorSet(frameIndex), nullptr);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

    commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

    // End rendering
    commandBuffer.endRendering();

    // Transition the image layout for presentation
    transitionImageLayout(
        swapchain->getImage(imageIndex),
        command->getCommandBuffer(frameIndex),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );

    commandBuffer.end();
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

void VulkanRenderer::updateUniformBuffer(std::uint32_t frameIndex)
{
    auto time = totalTimeMs / 1000.f;
    UniformBufferObject * ubo = new(uniformBuffersMapped[frameIndex]) UniformBufferObject;
    ubo->model = glm::ext::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo->view = glm::ext::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    auto [x,y] = window->getWindowClientAreaSize();
    ubo->proj = glm::ext::perspective(glm::radians(30.0f), static_cast<float>(x) / static_cast<float>(y), 0.1f, 10.0f);
    ubo->proj[1][1] *= -1;
    totalTimeMs += fpsCounter.getLastFrameTime();
}

void VulkanRenderer::init()
{
    window = std::make_unique<core::window::GLFWWindow>("Candela 3", 800, 600);
    window->registerWindowEvent(this);
    
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

    command = std::make_unique<VulkanCommand>(*device);
    command->init();

    createSyncObjects();

    // Vertex buffer
    auto vbSize = sizeof(vertices[0]) * vertices.size();
    copyDataToLocalDeviceMemory(*device, vertices.data(), vbSize, 
        vertexBuffer, vertexBufferMemory, vk::BufferUsageFlagBits::eVertexBuffer, 
        command->getCommandBuffer(0), device->getGraphicsQueue());
    
    // Index buffer
    auto ibSize = sizeof(indices[0]) * indices.size();
    copyDataToLocalDeviceMemory(*device, indices.data(), ibSize, 
        indexBuffer, indexBufferMemory, vk::BufferUsageFlagBits::eIndexBuffer, 
        command->getCommandBuffer(0), device->getGraphicsQueue());

    // Uniforms
    for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
        vk::raii::Buffer buffer({});
        vk::raii::DeviceMemory bufferMem({});
        createBuffer(*device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemory.emplace_back(std::move(bufferMem));
        uniformBuffersMapped.emplace_back( uniformBuffersMemory[i].mapMemory(0, bufferSize));
    }

    // Bind descriptor in set with buffers
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        auto &ds = pipeline->getDescriptorSet(i);
        vk::DescriptorBufferInfo bufferInfo{ .buffer = uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject) };
        vk::WriteDescriptorSet descriptorWrite{ 
            .dstSet = ds, .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1, 
            .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo 
        };
        device->getDevice().updateDescriptorSets(descriptorWrite, {});
    }
}

void VulkanRenderer::renderFrame()
{
    if (swapchainUnavailble)
        return;

    const auto frameMod = getFrameModulo();
    auto &drawFence = drawFences[frameMod];
    auto &presentCompleteSemaphore = presentCompleteSemaphores[frameMod];
    auto &commandBuffer = command->getCommandBuffer(frameMod);

    auto& dev = device->getDevice();
    dev.waitForFences(*drawFence, vk::True, std::numeric_limits<std::uint64_t>().max());
    
    auto [result, imageIndex] = swapchain->getSwapchain().acquireNextImage(std::numeric_limits<std::uint64_t>().max(), *presentCompleteSemaphore, nullptr);
    if (result == vk::Result::eErrorOutOfDateKHR) // we've been given an image, so do not recreate for vk::Result::eSuboptimalKHR at this instant
    {
        swapchain->recreate();
        return;
    }

    dev.resetFences(*drawFence);
    auto &renderFinishedSemaphore = renderFinishedSemaphores[imageIndex];

    // Record the actual drawing
    updateUniformBuffer(frameMod);
    recordCommandBuffer(imageIndex, frameMod);

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

    auto & graphicsQueue = device->getGraphicsQueue();
    graphicsQueue.submit(submitInfo, *drawFence);
    
    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapchain->getSwapchain(),
        .pImageIndices = &imageIndex
    };

    result = graphicsQueue.presentKHR(presentInfoKHR);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
        swapchain->recreate();
    ++frameNumber;
    if (fpsCounter.hitFrame())
        window->setWindowName(std::format("Candela 3 - Frames: {} FPS: {}", fpsCounter.getFrameCount(), fpsCounter.getFramesPerSecond()));
}

bool VulkanRenderer::processMessages() 
{
    return swapchainUnavailble ? window->waitMessages() : window->processMessages();
}

std::uint32_t VulkanRenderer::getFrameModulo() const
{
    return frameNumber % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::onResize(core::window::IWindow* window, int width, int height)
{
    swapchainUnavailble = width == 0 || height == 0;
    if (swapchainUnavailble)
        return;
    // Window resized, recreate swapchain
    swapchain->recreate();
    fpsCounter.resetFrameCount();
}