export module candela.renderer;

import std;
import core.window;
import vulkan;

export namespace candela::renderer
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual void init() = 0;
		virtual void renderFrame() = 0;
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
        std::vector<const char*> getRequiredInstanceExtensions();
        std::vector<const char*> getRequiredLayers();
        void setupDebugMessenger();
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

        vk::raii::Context  context;
        vk::raii::Instance instance;
        vk::raii::DebugUtilsMessengerEXT debugMessenger;
        vk::raii::PhysicalDevice physicalDevice;
        vk::raii::Device device;
        vk::raii::Queue graphicsQueue;
        std::uint32_t graphicsQueueFamilyIndex;
        vk::raii::SurfaceKHR surface;
        vk::raii::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Extent2D swapChainExtent;
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
        std::vector<vk::raii::ImageView> swapChainImageViews;
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline graphicsPipeline;

        static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;
        vk::raii::CommandPool commandPool;
        std::vector<vk::raii::CommandBuffer> commandBuffers;
        vk::raii::CommandBuffer *commandBuffer;

        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores; // based on swapchain frame index
        std::vector<vk::raii::Fence> drawFences;
        std::uint64_t frameIndex;

        // static constexpr bool enableValidationLayers;
        #ifdef NDEBUG
        static constexpr bool enableValidationLayers = false;
        #else
        static constexpr bool enableValidationLayers = true;
        #endif
    };
}