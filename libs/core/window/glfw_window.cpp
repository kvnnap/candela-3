module core.window;

import vulkan;

using namespace core::window;

static void handleGLFWError(int result, bool allowThrow = true)
{
    using namespace glfw;
    if (result != glfw_false)
        return;

    const char* description {};
    const auto code = glfwGetError(&description);
    if (code == glfw::glfw_no_error)
        return;
    auto errorCode = std::format("gflw_window(mine): code: {}, description: {}", code, description);
    if (allowThrow)
        throw std::runtime_error(errorCode);
    else 
        std::cerr << errorCode << std::endl;
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
    handleGLFWError(glfw::glfw_false, false);
}

bool GLFWWindow::glfwProcessMessages()
{
    glfw::glfwPollEvents();
    return windowCount == 0;
}

bool GLFWWindow::waitMessages()
{
    glfw::glfwWaitEvents();
    return windowCount == 0;
}

void GLFWWindow::waitUntilClientAreaExists()
{
    while (true)
    {
        auto [width, height] = getWindowClientAreaSize();
        if (width != 0 && height != 0)
            break;

        waitMessages();
    }
}

static void framebufferResizeCallback(glfw::GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<GLFWWindow*>(glfw::glfwGetWindowUserPointer(window));
    app->processResizeEvent(width, height);
}

// instance

GLFWWindow::GLFWWindow(const std::string& name, int width, int height)
    : window(), hasResized()
{
    using namespace glfw;
    init(); // static

    glfwWindowHint(glfw_client_api, glfw_no_api);
    glfwWindowHint(glfw_resizable, glfw_false);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    handleGLFWError(window == nullptr ? glfw_false : glfw_true);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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
    handleGLFWError(glfw::glfw_false, false);
    window = nullptr;
    destroy();
}

bool GLFWWindow::processMessages()
{
    glfwProcessMessages();
    return glfw::glfwWindowShouldClose(window);
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
    auto res = glfw::glfwCreateWindowSurface(*instance, window, nullptr, surface) == 0;
    handleGLFWError(res);
    return res;
}

void GLFWWindow::registerWindowEvent(IWindowEvent* windowEvent)
{
    windowEventCallbacks.emplace_back(windowEvent);
}

void GLFWWindow::processResizeEvent(int width, int height)
{
    for (auto wnd : windowEventCallbacks)
        wnd->onResize(this, width, height);
}
