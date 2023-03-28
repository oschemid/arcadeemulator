#include "engine.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"


using namespace ae::gui;


Engine::Engine(Window* window) :
    _window{ window },
    _graphicsQueueFamily{ 10000 }
{
}

Engine::~Engine() {
    for (auto& texture : _textures)
        freeTexture(texture.second);
    _textures.clear();

    _instance.destroySurfaceKHR(_surface);
    _allocator.destroy();
    _device.destroy();
    _instance.destroy();
}

void Engine::init()
{
	if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		throw std::exception("SDL Video is not initialized");
	init_vulkan();
}

void Engine::createInstance()
{
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(_window->get(), &extensions_count, NULL);
    const char** extensions = new const char* [extensions_count];
    SDL_Vulkan_GetInstanceExtensions(_window->get(), &extensions_count, extensions);

    vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Arcade Emulator",
        .applicationVersion = 1,
        .pEngineName = "xArcade",
        .engineVersion = 1,
        .apiVersion = VK_API_VERSION_1_3 };

    vk::InstanceCreateInfo instanceCreateInfo{ .pApplicationInfo = &applicationInfo, .enabledExtensionCount = extensions_count, .ppEnabledExtensionNames = extensions };
    _instance = vk::createInstance(instanceCreateInfo);
}

void Engine::selectGpu()
{
    // Select the first one
    _gpu = _instance.enumeratePhysicalDevices().front();

    // Select queue families
    std::vector<vk::QueueFamilyProperties> families = _gpu.getQueueFamilyProperties();
    for (size_t i = 0; i < families.size(); ++i) {
        if (families[i].queueFlags & vk::QueueFlagBits::eGraphics)
            _graphicsQueueFamily = static_cast<uint32_t>(i);
    }
    if (_graphicsQueueFamily == 10000)
        throw std::runtime_error("No graphics queue found");
}

void Engine::createDevice() {
    std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_QCOM_ROTATED_COPY_COMMANDS_EXTENSION_NAME };
    std::vector<float> queuePriority = { 1. };

    vk::DeviceQueueCreateInfo queueInfo{
        .queueFamilyIndex = _graphicsQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = queuePriority.data()
    };

    vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = extensions.data()
    };
    _device = _gpu.createDevice(createInfo);

    _graphicsQueue = _device.getQueue(_graphicsQueueFamily, 0);
}

void Engine::createAllocator()
{
    vma::AllocatorCreateInfo info{
        .physicalDevice = _gpu,
        .device = _device,
        .instance = _instance,
        .vulkanApiVersion = VK_API_VERSION_1_3
    };
    _allocator = vma::createAllocator(info);
}

void Engine::createSurfaceKHR()
{
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(_window->get(), _instance, &surface) == 0)
        throw std::exception("Failed to create Vulkan surface.\n");
    _surface = surface;

    // Select first compatible format 
    const vk::Format requestedFormat[] = { vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eB8G8R8Unorm };
    const vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
    auto availableFormat = _gpu.getSurfaceFormatsKHR(_surface);
    for (auto& av : availableFormat) {
        if (av.format == vk::Format::eUndefined) {
            _surfaceFormat.format = requestedFormat[0];
            _surfaceFormat.colorSpace = requestedColorSpace;
            break;
        }
        if (av.colorSpace == requestedColorSpace) {
            for (auto req : requestedFormat) {
                if (av.format == req) {
                    _surfaceFormat = av;
                    break;
                }
            }
        }
    }
}

void Engine::init_vulkan() {
	createInstance();
    selectGpu();
	createDevice();
    createAllocator();
    createSurfaceKHR();

    VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorpool);

    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
    _vulkanwindow.Surface = _surface;
    _vulkanwindow.SurfaceFormat = _surfaceFormat;
    
    _vulkanwindow.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(_gpu, _surface, present_modes, 1);

    ImGui_ImplVulkanH_CreateOrResizeWindow(_instance, _gpu, _device, &_vulkanwindow, _graphicsQueueFamily, nullptr, 1280, 800, 3);
}

void Engine::initImGui() {
    ImGui_ImplSDL2_InitForVulkan(_window->get());
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _instance;
    init_info.PhysicalDevice = _gpu;
    init_info.Device = _device;
    init_info.QueueFamily = _graphicsQueueFamily;
    init_info.Queue = _graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = _descriptorpool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 3;
    init_info.ImageCount = _vulkanwindow.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, _vulkanwindow.RenderPass);

    {
        // Use any command queue
        VkCommandPool command_pool = _vulkanwindow.Frames[_vulkanwindow.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer = _vulkanwindow.Frames[_vulkanwindow.FrameIndex].CommandBuffer;

        vkResetCommandPool(_device, command_pool, 0);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        vkEndCommandBuffer(command_buffer);
        vkQueueSubmit(_graphicsQueue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(_device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

void Engine::render(ImDrawData* data) {
    _vulkanwindow.ClearValue.color.float32[0] = 0;
    _vulkanwindow.ClearValue.color.float32[1] = 0;
    _vulkanwindow.ClearValue.color.float32[2] = 0;
    _vulkanwindow.ClearValue.color.float32[3] = 0;

    FrameRender(data);
    FramePresent();
}

void Engine::FrameRender(ImDrawData* data) {
    VkResult err;

    VkSemaphore image_acquired_semaphore = _vulkanwindow.FrameSemaphores[_vulkanwindow.SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = _vulkanwindow.FrameSemaphores[_vulkanwindow.SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(_device, _vulkanwindow.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &_vulkanwindow.FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        return;
    }
    ImGui_ImplVulkanH_Frame* fd = &_vulkanwindow.Frames[_vulkanwindow.FrameIndex];
    {
        err = vkWaitForFences(_device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
        err = vkResetFences(_device, 1, &fd->Fence);
    }
    {
        err = vkResetCommandPool(_device, fd->CommandPool, 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = _vulkanwindow.RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = _vulkanwindow.Width;
        info.renderArea.extent.height = _vulkanwindow.Height;
        info.clearValueCount = 1;
        info.pClearValues = &_vulkanwindow.ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        err = vkQueueSubmit(_graphicsQueue, 1, &info, fd->Fence);
    }
}

void Engine::FramePresent() {
    VkSemaphore render_complete_semaphore = _vulkanwindow.FrameSemaphores[_vulkanwindow.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &_vulkanwindow.Swapchain;
    info.pImageIndices = &_vulkanwindow.FrameIndex;
    VkResult err = vkQueuePresentKHR(_graphicsQueue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        return;
    }
    _vulkanwindow.SemaphoreIndex = (_vulkanwindow.SemaphoreIndex + 1) % _vulkanwindow.ImageCount; // Now we can use the next set of semaphores
}

RasterDisplay::RasterDisplay(Engine* engine) :
    _engine{ engine }
{}

RasterDisplay::~RasterDisplay()
{
    //_device.freeCommandBuffers(_pool, { _buffer });
    //_device.destroyCommandPool(_pool);
    //_device.freeMemory(_imageMemory);
    //_device.destroyImage(_image);
}

void RasterDisplay::init(const uint16_t width, const uint16_t height)
{
    _textureID = _engine->createTexture(width, height);
}

void RasterDisplay::refresh(uint8_t* src)
{
    _engine->fillTextureFromBuffer(_textureID, src);
}


RasterDisplay* Engine::getRasterDisplay() {
    return new RasterDisplay(this);
}

uint32_t Engine::findMemoryType(const uint32_t type, const vk::MemoryPropertyFlags properties)
{
    const vk::PhysicalDeviceMemoryProperties memoryproperties = _gpu.getMemoryProperties();
    uint32_t typeBits = type;
    for (uint32_t i = 0; i < memoryproperties.memoryTypeCount; ++i) {
        if ((typeBits & 1) && ((memoryproperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
        typeBits >>= 1;
    }
    return uint32_t(~0);
}
