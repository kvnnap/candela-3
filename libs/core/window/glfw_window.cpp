module core.window;

import vulkan;

using namespace core::window;

static void handleGLFWError(int result)
{
    using namespace glfw;
    if (result != glfw_false)
        return;

    const char* description {};
    const auto code = glfwGetError(&description);
    if (code == glfw::glfw_no_error)
        return;
    throw std::runtime_error("gflw_window(mine): code: " 
        + std::to_string(code) + ", description: " + std::string(description));
}

std::uint32_t GLFWWindow::windowCount = 0u;

void GLFWWindow::init()
{
    if (windowCount > 0)
        return;
    handleGLFWError(glfw::glfwInit());
    ++windowCount;
}

void GLFWWindow::destroy()
{
    if (windowCount == 0)
        return;
    --windowCount;
    if (windowCount > 0)
        return;
    glfw::glfwTerminate();
    handleGLFWError(glfw::glfw_false);
}

bool GLFWWindow::glfwProcessMessages()
{
    glfw::glfwPollEvents();

    return windowCount == 0;
}

// instance

GLFWWindow::GLFWWindow(const std::string& name, int width, int height)
    : window()
{
    using namespace glfw;
    init(); // static

    glfwWindowHint(glfw_client_api, glfw_no_api);
    glfwWindowHint(glfw_resizable, glfw_false);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    handleGLFWError(window == nullptr ? glfw_false : glfw_true);
}

GLFWWindow::~GLFWWindow()
{
    cleanup();
}

void GLFWWindow::cleanup()
{
    if (window == nullptr)
        return;
    glfw::glfwDestroyWindow(window);
    window = nullptr;
    destroy();
}

bool GLFWWindow::processMessages()
{
    glfwProcessMessages();
    auto res = glfw::glfwWindowShouldClose(window);
    if (res)
        cleanup();
    return res;
}

std::pair<int, int> GLFWWindow::getWindowClientAreaSize() const
{
    std::pair<int, int> p;
    glfw::glfwGetFramebufferSize(window, &p.first, &p.second);
    return p;
}

std::vector<const char*> GLFWWindow::getRequiredVulkanExtensions() const
{
    std::uint32_t glfwExtensionCount{};
    auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    return std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

bool GLFWWindow::createVulkanSurface(const void* vkInstance, void* vkSurfaceKHR)
{
    const auto instance = static_cast<const vk::raii::Instance::CType*>(vkInstance);
    auto surface = static_cast<vk::raii::SurfaceKHR::CType *>(vkSurfaceKHR);
    return glfw::glfwCreateWindowSurface(*instance, window, nullptr, surface) == 0;
}