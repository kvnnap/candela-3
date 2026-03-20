export module candela.renderer;

import std;
import core.window;
import vulkan;

export namespace candela::renderer
{
        // static constexpr bool enableValidationLayers;
    #ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
    #else
    constexpr bool enableValidationLayers = true;
    #endif

    auto extComparator = [](const vk::ExtensionProperties& e, const char* r){ return std::string_view(e.extensionName) == r; };

	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual void init() = 0;
		virtual void renderFrame() = 0;
	};

    class VulkanInstance
    {
    public:
        VulkanInstance(const core::window::IVulkanWindow& window);
        void init();

        // Getters
        vk::raii::Instance& getInstance();
    private:
        std::vector<const char*> getRequiredInstanceExtensions();
        std::vector<const char*> getRequiredLayers();
        void setupDebugMessenger();

        const core::window::IVulkanWindow& window;
        vk::raii::Context context; 
        vk::raii::Instance instance; // needs context
        vk::raii::DebugUtilsMessengerEXT debugMessenger; // needs instance
    };

    class VulkanRenderer
        : public IRenderer
    {
    public:
        VulkanRenderer();
        ~VulkanRenderer() override;
        void init() override;
        void renderFrame() override;

        bool processMessages();
    private:

        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createImageViews();
        vk::raii::ShaderModule createShaderModule(const std::vector<std::byte>& code);
        void createGraphicsPipeline();
        void createCommandPool();
        void createCommandBuffer();
        void transitionImageLayout(std::uint32_t imageIndex, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask);
        
        void recordCommandBuffer(std::uint32_t imageIndex);

        void createSyncObjects();

        // static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
        //                                               vk::DebugUtilsMessageTypeFlagsEXT              type,
        //                                               const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        //                                               void *                                         pUserData);

        std::uint32_t getFrameModulo() const;

        std::unique_ptr<core::window::IVulkanWindow> window;

        std::unique_ptr<VulkanInstance> instance;

        vk::raii::PhysicalDevice physicalDevice; // needs instance
        vk::raii::Device device; // needs physical device

        vk::raii::Queue graphicsQueue; // needs device
        std::uint32_t graphicsQueueFamilyIndex; // From physical device

        // Swap chain + window + surface related stuff
        vk::raii::SurfaceKHR surface; // needs instance + window
        vk::raii::SwapchainKHR swapChain; // needs device + surface
        std::vector<vk::Image> swapChainImages; // needs swapchain
        vk::Extent2D swapChainExtent; // needs surface window
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
        std::vector<vk::raii::ImageView> swapChainImageViews; // needs swapchain


        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline graphicsPipeline;

        static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        vk::raii::CommandPool commandPool;
        std::vector<vk::raii::CommandBuffer> commandBuffers;
        vk::raii::CommandBuffer *commandBuffer;

        // Sync stuff
        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores; // based on swapchain frame index
        std::vector<vk::raii::Fence> drawFences;
        std::uint64_t frameIndex;
    };
}