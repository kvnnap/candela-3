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

    template<typename T>
    void setDebugName(const vk::raii::Device& device, T& obj, const std::string& name)
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfo {
            .objectType = T::objectType,
            .objectHandle = reinterpret_cast<std::uintptr_t>(static_cast<T::CType>(*obj)),
            .pObjectName = name.c_str()
        };
        device.setDebugUtilsObjectNameEXT(nameInfo);
    }

    constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    void transitionImageLayout(const vk::Image& image, vk::raii::CommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask);

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
        const vk::raii::Instance& getInstance() const;
    private:
        std::vector<const char*> getRequiredInstanceExtensions();
        std::vector<const char*> getRequiredLayers();
        void setupDebugMessenger();

        const core::window::IVulkanWindow& window;

        vk::raii::Context context; 
        vk::raii::Instance instance; // needs context
        vk::raii::DebugUtilsMessengerEXT debugMessenger; // needs instance
    };

    class VulkanSwapchain;

    class VulkanDevice
    {
    public:
        VulkanDevice(const VulkanInstance& instance, const VulkanSwapchain& swapchain);
        
        void init();
        void wait() const;

        const vk::raii::PhysicalDevice& getPhysicalDevice() const;
        const vk::raii::Device& getDevice() const;
        const vk::raii::Queue& getGraphicsQueue() const;
        std::uint32_t getGraphicsQueueFamilyIndex() const;
    private:
        void pickPhysicalDevice(); // needs surface (to query for support)
        void createLogicalDevice();

        const VulkanInstance& instance;
        const VulkanSwapchain& swapchain;

        vk::raii::PhysicalDevice physicalDevice; // needs instance
        vk::raii::Device device; // needs physical device
        vk::raii::Queue graphicsQueue; // needs device
        std::uint32_t graphicsQueueFamilyIndex; // From physical device
    };

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(core::window::IVulkanWindow &window, const VulkanInstance& instance);
        
        const vk::raii::SurfaceKHR& getSurface() const;
        const vk::raii::SwapchainKHR& getSwapchain() const;
        vk::Format getFormat() const;
        

        const vk::Image& getImage(std::size_t index) const;
        const vk::raii::ImageView& getImageView(std::size_t index) const;
        const vk::Extent2D& getExtent() const;
        std::size_t getSize() const;

        void setDevice(const VulkanDevice& device);
        
        void initSurface();
        void initSwapchain();
        void recreate();

    private:
        void createSurface();
        void createSwapChain();
        void createImageViews();

        // Non-owning references
        core::window::IVulkanWindow &window;
        const VulkanInstance& instance;
        const VulkanDevice* device;

        // Swap chain + window + surface related stuff
        vk::raii::SurfaceKHR surface; // needs instance + window
        vk::raii::SwapchainKHR swapChain; // needs device + surface
        std::vector<vk::Image> swapChainImages; // needs swapchain
        std::vector<vk::raii::ImageView> swapChainImageViews; // needs swapchain
        vk::Extent2D swapChainExtent; // needs surface window
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
    };

    class VulkanPipeline
    {
    public:
        VulkanPipeline(const VulkanDevice& device, const VulkanSwapchain& swapchain);
        void init();

        const vk::raii::Pipeline& getGraphicsPipeline() const;
    private:
        vk::raii::ShaderModule createShaderModule(const std::vector<std::byte>& code);
        void createGraphicsPipeline();

        const VulkanDevice& device;
        const VulkanSwapchain& swapchain;
        
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline graphicsPipeline;
    };

    class VulkanCommand
    {
    public:
        VulkanCommand(const VulkanDevice& device);
        void init();

        vk::raii::CommandBuffer& getCommandBuffer(std::uint32_t frameIndex);
    private:
        void createCommandPool();
        void createCommandBuffer();

        const VulkanDevice& device;

        vk::raii::CommandPool commandPool;
        std::vector<vk::raii::CommandBuffer> commandBuffers;
    };

    class VulkanRenderer
        : public IRenderer, public core::window::IWindowEvent
    {
    public:
        VulkanRenderer();
        ~VulkanRenderer() override;
        void init() override;
        void renderFrame() override;

        bool processMessages();
    private:

        void onResize(core::window::IWindow* window, int width, int height) override;
        
        void recordCommandBuffer(std::uint32_t imageIndex, std::uint32_t frameIndex);

        void createSyncObjects();

        // static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
        //                                               vk::DebugUtilsMessageTypeFlagsEXT              type,
        //                                               const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        //                                               void *                                         pUserData);

        std::uint32_t getFrameModulo() const;

        std::unique_ptr<core::window::IVulkanWindow> window;

        std::unique_ptr<VulkanInstance> instance;
        std::unique_ptr<VulkanDevice> device;
        std::unique_ptr<VulkanSwapchain> swapchain;
        std::unique_ptr<VulkanPipeline> pipeline;
        std::unique_ptr<VulkanCommand> command;

        // Sync stuff
        std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
        std::vector<vk::raii::Semaphore> renderFinishedSemaphores; // based on swapchain frame index
        std::vector<vk::raii::Fence> drawFences;
        std::uint64_t frameNumber;
        bool swapchainUnavailble; // rendering is paused
    };
}