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


        // static VKAPI_ATTR vk::Bool32 VKAPI_CALL vkDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
        //                                               vk::DebugUtilsMessageTypeFlagsEXT              type,
        //                                               const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
        //                                               void *                                         pUserData);

        glfw::GLFWwindow* window;

        vk::raii::Context  context;
        vk::raii::Instance instance;
        vk::raii::DebugUtilsMessengerEXT debugMessenger;
        vk::raii::PhysicalDevice physicalDevice;

        bool enableValidationLayers;
    };
}