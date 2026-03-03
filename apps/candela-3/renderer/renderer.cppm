module;

#include "vkheader.h"

export module candela.renderer;

import std;
import external.glfw;

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
        glfw::GLFWwindow* window;

        vk::raii::Context  context;
        vk::raii::Instance instance = nullptr;
    };
}