#include <vulkan/vulkan.hpp>
#include "gpu.h"

class GpuVulkan : public Gpu
{
	public:
		void render() const override;
		GpuVulkan(Window* w);
		~GpuVulkan();
	private:
        struct SwapChainFrame
        {
            vk::UniqueImage image;
            vk::UniqueImageView imageView;
            vk::UniqueFramebuffer frameBuffer;
            vk::UniqueCommandBuffer commandBuffer;
        };
		
        vk::UniqueInstance instance;
		vk::PhysicalDevice physicalDevice;
		vk::UniqueDevice device;
		vk::UniqueSurfaceKHR surface;
		vk::UniqueSwapchainKHR swapChain;
        vk::Format swapChainImgFormat;
	    vk::Extent2D extent;
        vk::UniqueRenderPass renderPass;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipeline graphicsPipeline;
        vk::UniqueCommandPool commandPool;

        std::vector<std::unique_ptr<SwapChainFrame>> frames;
		std::vector<const char*> validationLayers;

		struct
		{
			int graphics{-1};
			int present{-1};
			int compute{-1};
		} queueFamilyIDs;

        struct
        {
            vk::UniqueSemaphore imgReady;
            vk::UniqueSemaphore renderReady;
        } semaphores;

        struct
        {
            vk::Queue graphics;
    		vk::Queue compute;
    		vk::Queue present;
        } queues;

        std::vector<char> loadShader(const char* path);
        vk::UniqueShaderModule createShaderModule(std::vector<char> source);
		void createInstance();
		void selectPhysicalDevice();
		void createDevice();
		void createSurface();
		void createSwapChain();
		void createSwapChainImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createSemaphores();
		bool isDeviceOK(const vk::PhysicalDevice &potDevice);
};
