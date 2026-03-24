module candela.renderer;

using candela::renderer::VulkanSwapchain;

VulkanSwapchain::VulkanSwapchain(core::window::IVulkanWindow &window, const VulkanInstance& instance)
    : window(window), instance(instance), device(),
      surface(nullptr), swapChain(nullptr), swapChainExtent(),
      swapChainSurfaceFormat()
{}

void VulkanSwapchain::setDevice(const VulkanDevice& device)
{ 
    this->device = &device;
}

const vk::raii::SurfaceKHR& VulkanSwapchain::getSurface() const { return surface; }
const vk::raii::SwapchainKHR& VulkanSwapchain::getSwapchain() const { return swapChain; }
vk::Format VulkanSwapchain::getFormat() const { return swapChainSurfaceFormat.format; }
const vk::Image& VulkanSwapchain::getImage(std::size_t index) const { return swapChainImages[index]; }
const vk::raii::ImageView& VulkanSwapchain::getImageView(std::size_t index) const { return swapChainImageViews[index]; }
const vk::Extent2D& VulkanSwapchain::getExtent() const { return swapChainExtent; }
std::size_t VulkanSwapchain::getSize() const { return swapChainImages.size(); }

void VulkanSwapchain::initSurface()
{
    createSurface();
}

void VulkanSwapchain::initSwapchain()
{
    swapChainImageViews.clear();
    swapChainImages.clear();
    swapChain = nullptr;
    
    createSwapChain();
    createImageViews();
}

void VulkanSwapchain::recreate()
{
    // window.waitUntilClientAreaExists();
    device->wait();
    initSwapchain();
}

void VulkanSwapchain::createSurface()
{
    vk::raii::SurfaceKHR::CType _surface{nullptr};
    auto &inst = instance.getInstance();
    if(!window.createVulkanSurface(&*inst, &_surface))
        throw std::runtime_error("failed to create window surface!");
    surface = vk::raii::SurfaceKHR(inst, _surface);
}

void VulkanSwapchain::createSwapChain()
{
    const auto& physicalDevice = device->getPhysicalDevice();
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
        auto [width, height] = window.getWindowClientAreaSize();
        extent.width = std::clamp<std::uint32_t>(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        extent.height = std::clamp<std::uint32_t>(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
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

    swapChain = vk::raii::SwapchainKHR(device->getDevice(), swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
}

void VulkanSwapchain::createImageViews()
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
        swapChainImageViews.emplace_back(device->getDevice(), imageViewCreateInfo);
    }
}
