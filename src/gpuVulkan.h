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
            vk::Image image;
            vk::ImageView imageView;
            vk::Framebuffer frameBuffer;
        };

		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue computeQueue;
		vk::Queue presentQueue;
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapChain;
	    vk::Extent2D extent;
        vk::RenderPass renderPass;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;
        vk::CommandPool commandPool;

        std::vector<SwapChainFrame> frames;
        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImageViews;
        std::vector<vk::Framebuffer> swapChainFramebuffers;
        std::vector<vk::CommandBuffer> commandBuffers;
		std::vector<const char*> validationLayers;
		struct
		{
			int graphics{-1};
			int present{-1};
			int compute{-1};
		} queueFamilyIDs;
        std::vector<char> loadShader(const char* path);
        vk::ShaderModule createShaderModule(std::vector<char> source);
		void createInstance();
		void selectPhysicalDevice();
		void createDevice();
		void createSurface();
		void createSwapChain();
		void createSwapChainImageViews(vk::Format format);
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
		bool isDeviceOK(const vk::PhysicalDevice &potDevice);
};
