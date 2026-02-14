module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

import vulkan;

export module hello_triangle_app;

export class HelloTriangleApp {
  public:
    void Run() {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

  private:
    void InitWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window_ = glfwCreateWindow(800, 600, "Domeykite", nullptr, nullptr);
    }

    void InitVulkan() { CreateInstance(); }

    void CreateInstance() {
        context_ = vk::raii::Context{};

        constexpr vk::ApplicationInfo app_info{.pApplicationName = "Hello Triangle",
                                               .applicationVersion = vk::makeVersion(1, 0, 0),
                                               .pEngineName = "No Engine",
                                               .engineVersion = vk::makeVersion(1, 0, 0),
                                               .apiVersion = vk::ApiVersion14};

        // Get required extensions from GLFW.
        uint32_t glfw_extension_count = 0;
        const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        // Check if the required extensions are supported by the Vulkan implementation.
        const auto extension_properties = context_.enumerateInstanceExtensionProperties();

        for (uint32_t i = 0; i < glfw_extension_count; i++) {
            if (std::ranges::none_of(extension_properties,
                                     [glfw_extension = glfw_extensions[i]](const auto &extension_property) -> bool {
                                         return std::strcmp(extension_property.extensionName, glfw_extension) == 0;
                                     })) {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfw_extensions[i]));
            }
        }

        vk::InstanceCreateInfo createInfo{.pApplicationInfo = &app_info,
                                          .enabledExtensionCount = glfw_extension_count,
                                          .ppEnabledExtensionNames = glfw_extensions};

        instance_ = vk::raii::Instance{context_, createInfo};
    }

    void MainLoop() {
        while (!glfwWindowShouldClose(window_)) {
            glfwPollEvents();
        }
    }

    void Cleanup() {
        glfwDestroyWindow(window_);

        glfwTerminate();
    }

    GLFWwindow *window_{nullptr};
    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
};
