module candela.renderer;

import core.util;

using candela::renderer::VulkanCommand;

VulkanCommand::VulkanCommand(const VulkanDevice& device)
    : device(device), commandPool(nullptr)
{}

void VulkanCommand::init()
{
    createCommandPool();
    createCommandBuffer();
}

vk::raii::CommandBuffer& VulkanCommand::getCommandBuffer(std::uint32_t frameIndex)
{
    return commandBuffers.at(frameIndex);
}

void VulkanCommand::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ 
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, 
        .queueFamilyIndex = device.getGraphicsQueueFamilyIndex() 
    };
    commandPool = vk::raii::CommandPool(device.getDevice(), poolInfo);
}

void VulkanCommand::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo { 
        .commandPool = commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };
    commandBuffers = vk::raii::CommandBuffers(device.getDevice(), allocInfo);
    if constexpr(enableValidationLayers)
    {
        for (auto i = 0u; i < MAX_FRAMES_IN_FLIGHT; ++i)
            setDebugName(device.getDevice(), commandBuffers[i], std::format("commandBuffers[{}]", i));
    }
}