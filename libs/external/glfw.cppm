module;

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

export module external.glfw;

export namespace glfw
{
    using ::GLFWwindow;
    using ::glfwInit;
    using ::glfwWindowShouldClose;
    using ::glfwPollEvents;
    using ::glfwWindowHint;
    using ::glfwCreateWindow;
    using ::glfwDestroyWindow;
    using ::glfwTerminate;
    using ::glfwGetRequiredInstanceExtensions;
    using ::glfwGetError;
    using ::glfwCreateWindowSurface;

    constexpr auto glfw_client_api = GLFW_CLIENT_API;
    constexpr auto glfw_no_api = GLFW_NO_API;
    constexpr auto glfw_resizable = GLFW_RESIZABLE;
    constexpr auto glfw_true = GLFW_TRUE;
    constexpr auto glfw_false = GLFW_FALSE;
}
