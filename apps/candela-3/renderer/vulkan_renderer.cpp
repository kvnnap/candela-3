module candela.renderer;

import vulkan;

using candela::renderer::VulkanRenderer;

VulkanRenderer::VulkanRenderer()
{
    init();
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::init()
{
    vk::Instance instance;
    vk::ApplicationInfo appInfo{
        .pApplicationName = "Candela",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo
    };

    // instance = vk::raii::Instance();
}

void VulkanRenderer::renderFrame()
{

}
