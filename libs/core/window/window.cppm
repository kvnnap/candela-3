export module core.window;

import std;
import external.glfw;

export namespace core::window
{

class IWindow
{
public:
    virtual ~IWindow() = default;
    virtual bool processMessages() = 0;
    virtual std::pair<int, int> getWindowClientAreaSize() const = 0;
};

class IVulkanWindow
    : public IWindow
{
public:
    virtual ~IVulkanWindow() = default;
    virtual std::vector<const char*> getRequiredVulkanExtensions() const = 0;
    virtual bool createVulkanSurface(const void* vkInstance, void* vkSurfaceKHR) = 0; // This should probably move out?
};

class GLFWWindow
    : public IVulkanWindow
{
public:
    GLFWWindow(const std::string& name, int width, int height);
    ~GLFWWindow() override;
    void cleanup();
    bool processMessages() override;
    std::pair<int, int> getWindowClientAreaSize() const override;
    std::vector<const char*> getRequiredVulkanExtensions() const override;
    bool createVulkanSurface(const void* vkInstance, void* vkSurfaceKHR) override;
    static bool glfwProcessMessages();
private:
    glfw::GLFWwindow *window;
    static std::uint32_t windowCount;
    static void init();
    static void destroy();
};

}
