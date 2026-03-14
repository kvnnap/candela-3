export module candela.renderer;

import std;
import external.glfw;
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

        void initWindow();
        std::optional<bool> processMessages();
        void cleanup();
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
        


        // static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
        //                                               vk::DebugUtilsMessageTypeFlagsEXT              type,
        //                                               const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        //                                               void *                                         pUserData);

        glfw::GLFWwindow* window;

        vk::raii::Context  context;
        vk::raii::Instance instance;
        vk::raii::DebugUtilsMessengerEXT debugMessenger;
        vk::raii::PhysicalDevice physicalDevice;
        vk::raii::Device device;
        vk::raii::Queue graphicsQueue;
        vk::raii::SurfaceKHR surface;
        vk::raii::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Extent2D swapChainExtent;
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
        std::vector<vk::raii::ImageView> swapChainImageViews;
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline graphicsPipeline;

        bool enableValidationLayers;
    };
}