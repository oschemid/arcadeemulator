#pragma once
#include "types.h"
#define VULKAN_HPP_NO_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.hpp"
#include "SDL2/SDL_vulkan.h"

#include "SDL2/SDL.h"
#include "imgui_impl_vulkan.h"


namespace ae::gui {
	class Engine;

	class Window {
	protected:
		string _title;
		uint16_t _width;
		uint16_t _height;
		SDL_Window* _window;

	public:
		Window(const string&, const uint16_t, const uint16_t);
		void init();

		SDL_Window* get() const { return _window; }
	};

	class Engine {
	protected:
		struct Texture
		{
		public:
			vk::Extent2D size;
			vk::Buffer buffer;
			vma::Allocation bufferAllocation;
			vk::Image image;
			vma::Allocation imageAllocation;
			vk::ImageView imageView;
			vk::Sampler imageSampler;
			vk::CommandPool commandPool;
			vk::CommandBuffer commandBuffer;
			vk::DescriptorSet descriptorID;
		};

	protected:
		Window* _window;

		// Instance attributes
		vk::Instance _instance;

		// Gpu attributes
		vk::PhysicalDevice _gpu;
		uint32_t _graphicsQueueFamily;

		// Device attributes
		vk::Device _device;
		vk::Queue _graphicsQueue;

		// Memory Allocator attributes
		vma::Allocator _allocator;

		// SurfaceKHR attributes
		vk::SurfaceKHR _surface;
		vk::SurfaceFormatKHR _surfaceFormat;



		VkDescriptorPool _descriptorpool;

		std::map<ImTextureID, Texture> _textures;
		ImGui_ImplVulkanH_Window _vulkanwindow;

		void init_vulkan();

		void createInstance();
		void selectGpu();
		void createDevice();
		void createAllocator();
		void createSurfaceKHR();

		void freeTexture(const Texture&);

		void FrameRender(ImDrawData*);
		void FramePresent();

		uint32_t findMemoryType(const uint32_t, const vk::MemoryPropertyFlags);
	public:
		Engine(Window*);
		~Engine();

		void init();
		void initImGui();

		ImTextureID createTexture(const uint16_t, const uint16_t);
		ImTextureID createTextureFromFile(const string&);
		void removeTexture(const ImTextureID&);
		void fillTextureFromBuffer(const ImTextureID&, const unsigned char*);

		void render(ImDrawData*);
	};
}