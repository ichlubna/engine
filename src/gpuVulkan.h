#include <vulkan/vulkan.hpp>
#include "gpu.h"

class GpuVulkan : public Gpu
{
	public:
		void render() override;
		GpuVulkan(Window* w);
		~GpuVulkan();
	private: 
        const struct
        {
            unsigned int position{0};
            unsigned int normal{1};
            unsigned int uv{2};
        } locations;

        struct SwapChainFrame
        {
            vk::Image image;
            vk::UniqueImageView imageView;
            vk::UniqueFramebuffer frameBuffer;
            vk::UniqueCommandBuffer commandBuffer;
        };
        
        struct PipelineSync
        {
            struct
            {
                vk::UniqueSemaphore imgReady;
                vk::UniqueSemaphore renderReady;
            } semaphores;
            vk::UniqueFence fence;
        };
 
        const int CONCURRENT_FRAMES_COUNT = 2;
        unsigned int processedFrame = 0;
		
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
        std::vector<PipelineSync> pipelineSync;

		struct
		{
			int graphics{-1};
			int present{-1};
			int compute{-1};
		} queueFamilyIDs;

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
        void recreateSwapChain();
		void createSwapChainImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createPipelineSync();
		bool isDeviceOK(const vk::PhysicalDevice &potDevice);
};
