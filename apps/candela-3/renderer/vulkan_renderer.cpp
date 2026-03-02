module candela.renderer;

// import vulkan;
// import external.glfw;

using candela::renderer::VulkanRenderer;

VulkanRenderer::VulkanRenderer()
{
    // init();
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::init()
{
    vk::Instance instance;
    vk::ApplicationInfo appInfo{
        .pApplicationName = "Candela",
        .applicationVersion = vk::makeVersion(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = vk::makeVersion(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo
    };

    // instance = vk::raii::Instance();
    // Window

    // Query extensions
    std::uint32_t glfwExtensionCount{};
    auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    // Extensions supported by Vulkan?
    // auto extensionProperties = context.enumerateInstanceExtensionProperties();
    // for (auto i = 0u; i < glfwExtensionCount; ++i)
    // {
    //     const auto res = std::ranges::none_of(extensionProperties,
    //         [glfwExtension = glfwExtensions[i]](const auto& extensionProperty)
    //         { 
    //             return std::strcmp(extensionProperty.extensionName, glfwExtension) == 0; 
    //         });
    //     if (res)
    //         throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
    // }
}

void VulkanRenderer::initWindow()
{
    using namespace glfw;
    constexpr unsigned WIDTH = 800;
    constexpr unsigned HEIGHT = 600;

    glfwInit();

    glfwWindowHint(glfw_client_api, glfw_no_api);
    glfwWindowHint(glfw_resizable, glfw_false);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Kevin", nullptr, nullptr);
}


void VulkanRenderer::renderFrame()
{
    
}

std::optional<bool> VulkanRenderer::processMessages() 
{
    using namespace glfw;

    int shouldClose {};
    while (!(shouldClose = glfwWindowShouldClose(window))) {
        glfwPollEvents();
    }
    return shouldClose;
}

void VulkanRenderer::cleanup()
{
    using namespace glfw;
    glfwDestroyWindow(window);
    glfwTerminate();
}