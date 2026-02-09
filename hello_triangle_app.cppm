module;

import vulkan;

export module hello_triangle_app;

export class HelloTriangleApp
{
public:
    void run()
    {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan()
    {
        context = vk::raii::Context{};

        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Hello Triangle",
            .applicationVersion = vk::makeVersion(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = vk::makeVersion(1, 0, 0),
            .apiVersion = vk::ApiVersion14};

        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo};

        instance = vk::raii::Instance{context, createInfo};
    }

    void mainLoop()
    {
    }

    void cleanup()
    {
    }

    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};
};
