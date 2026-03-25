module candela.renderer;

void candela::renderer::transitionImageLayout(const vk::Image& image, vk::raii::CommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask)
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
        .image = image,
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

void candela::renderer::createBuffer(const candela::renderer::VulkanDevice& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory)
{
    auto& dev = device.getDevice();
    auto findMemoryType = [&](std::uint32_t typeFilter, vk::MemoryPropertyFlags properties){
        vk::PhysicalDeviceMemoryProperties memProperties = device.getPhysicalDevice().getMemoryProperties();
        for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        return memProperties.memoryTypeCount;
    };
    vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
    buffer = vk::raii::Buffer(dev, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
    bufferMemory = vk::raii::DeviceMemory(dev, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
}

// blocks until transfer is complete
void candela::renderer::copyDataToLocalDeviceMemory(const candela::renderer::VulkanDevice& device, 
    const void* sData, std::size_t dSize, vk::raii::Buffer& dBuffer, vk::raii::DeviceMemory& dBufferMemory, 
    vk::BufferUsageFlags dstUsage, const vk::raii::CommandBuffer& cmd, const vk::raii::Queue& copyQueue)
{
    vk::raii::Buffer stagingBuffer = nullptr;
    vk::raii::DeviceMemory stagingBufferMemory = nullptr;
    createBuffer(device, dSize, vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible |  vk::MemoryPropertyFlagBits::eHostCoherent, 
        stagingBuffer, stagingBufferMemory);
    void *data = stagingBufferMemory.mapMemory(0, dSize);
    std::memcpy(data, sData, dSize);
    stagingBufferMemory.unmapMemory();

    createBuffer(device, dSize, dstUsage | vk::BufferUsageFlagBits::eTransferDst, 
        vk::MemoryPropertyFlagBits::eDeviceLocal, dBuffer, dBufferMemory);
    
    cmd.begin({});
    cmd.copyBuffer(stagingBuffer, dBuffer, vk::BufferCopy(0, 0, dSize));
    cmd.end();
    copyQueue.submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmd }, nullptr);
    copyQueue.waitIdle();
}

