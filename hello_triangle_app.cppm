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

    void InitVulkan() {
        CreateInstance();
        SetupDebugMessenger();
        PickPhysicalDevice();
    }

    void CreateInstance() {
        context_ = vk::raii::Context{};

        constexpr vk::ApplicationInfo app_info{.pApplicationName = "Hello Triangle",
                                               .applicationVersion = vk::makeVersion(1, 0, 0),
                                               .pEngineName = "No Engine",
                                               .engineVersion = vk::makeVersion(1, 0, 0),
                                               .apiVersion = vk::ApiVersion14};

        // Get required validation layers if enabled.
        const std::vector<char const *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        std::vector<char const *> required_layers;

        if (enable_validation_layers_) {
            required_layers.assign(validation_layers.begin(), validation_layers.end());
        }

        // Chceck if the required validation layers are supported by the Vulkan implementation.
        const auto layer_properties = context_.enumerateInstanceLayerProperties();
        if (std::ranges::any_of(required_layers, [&layer_properties](const auto &required_layer) -> bool {
                return std::ranges::none_of(layer_properties, [required_layer](const auto &layer_property) -> bool {
                    return std::strcmp(layer_property.layerName, required_layer) == 0;
                });
            })) {
            throw std::runtime_error("One or more required validation layers are not supported.");
        }

        const auto extensions = GetRequiredExtensions();
        const vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<uint32_t>(required_layers.size()),
            .ppEnabledLayerNames = required_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        instance_ = vk::raii::Instance{context_, createInfo};
    }

    std::vector<const char *> GetRequiredExtensions() {
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

        std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (enable_validation_layers_) {
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        return extensions;
    }

    void SetupDebugMessenger() {
        if (!enable_validation_layers_) {
            return;
        }

        constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity_flags =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        constexpr vk::DebugUtilsMessageTypeFlagsEXT type_flags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        constexpr vk::DebugUtilsMessengerCreateInfoEXT create_info{
            .messageSeverity = severity_flags,
            .messageType = type_flags,
            .pfnUserCallback = &DebugCallback,
        };

        debug_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info);
    }

    void PickPhysicalDevice() {
        const auto physical_devices = instance_.enumeratePhysicalDevices();

        const auto physical_device_it =
            std::ranges::find_if(physical_devices, [&](const auto &physical_device) -> bool {
                const auto queue_family_properties = physical_device.getQueueFamilyProperties();
                bool is_suitable = physical_device.getProperties().apiVersion >= vk::ApiVersion13;

                const auto queue_family_property_it =
                    std::ranges::find_if(queue_family_properties, [](const auto &queue_family_property) -> bool {
                        return (queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics) != vk::QueueFlags{0};
                    });

                is_suitable = is_suitable && (queue_family_property_it != queue_family_properties.end());

                const auto extension_properties = physical_device.enumerateDeviceExtensionProperties();
                bool found = true;
                for (const auto &extension : extension_properties) {
                    const auto extension_iter =
                        std::ranges::find_if(extension_properties, [extension](const auto &extension_property) -> bool {
                            return std::strcmp(extension_property.extensionName, extension.extensionName) == 0;
                        });
                    found = found && (extension_iter != extension_properties.end());
                }

                is_suitable = is_suitable && found;

                if (is_suitable) {
                    std::cout << "Selected GPU: " << physical_device.getProperties().deviceName << std::endl;
                    physical_device_ = physical_device;
                }

                return is_suitable;
            });

        if (physical_device_it == physical_devices.end()) {
            throw std::runtime_error("Failed to find a suitable GPU.");
        }
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

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT *callback_data,
                                                          void *) {
        std::cerr << "validation layer: type " << vk::to_string(type) << " msg " << callback_data->pMessage
                  << std::endl;

        return vk::False;
    }

#ifdef NDEBUG
    const bool enable_validation_layers_ = false;
#else
    const bool enable_validation_layers_ = true;
#endif

    GLFWwindow *window_{nullptr};

    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
    vk::raii::PhysicalDevice physical_device_{nullptr};
};
