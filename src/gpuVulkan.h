#include <vulkan/vulkan.hpp>
#include "gpu.h"

class GpuVulkan : public Gpu
{
	public:
		void render() override;
		void updateViewProjectionMatrix(glm::mat4 view) override;
        void addModel(std::shared_ptr<Assets::Model> model) override;
        void addTexture(std::shared_ptr<Assets::Texture> texture) override;
		GpuVulkan(Window* w);
		~GpuVulkan();
	private: 
        const struct
        {
            unsigned int position{0};
            unsigned int normal{1};
            unsigned int uv{2};
        } locations;

        const struct
        {
            unsigned int viewProjectionMatrix{0};
            unsigned int texture{1};
            unsigned int sampler{2};
        } bindings;

        struct PipelineSync
        {
            struct
            {
                vk::UniqueSemaphore imgReady;
                vk::UniqueSemaphore renderReady;
            } semaphores;
            vk::UniqueFence fence;
        };  
  
        struct Buffer
        {
            vk::UniqueBuffer buffer;
            vk::UniqueDeviceMemory memory;
            unsigned int top{0};
        };

        struct Image
        {
            vk::UniqueImage textureImage;
            vk::UniqueDeviceMemory textureImageMemory;
        };
        
        struct Texture
        {
            Image image;
            vk::UniqueImageView imageView;
        };

        struct SwapChainFrame
        {
            vk::Image image;
            vk::UniqueImageView imageView;
            vk::UniqueFramebuffer frameBuffer;
            vk::UniqueCommandBuffer commandBuffer;
            Buffer uniformVpMatrix;
            vk::UniqueDescriptorSet descriptorSet;
        };
        
        struct Textures
        {
            static constexpr int WIDTH{150};
            static constexpr int HEIGHT{150};
            static constexpr int MAX_COUNT{5};
            unsigned int top{0}; 
            Texture images[MAX_COUNT];
        };
 
        const int CONCURRENT_FRAMES_COUNT = 2;
        //TODO 100???
        const int VERTEX_BUFFER_SIZE{100*sizeof(Assets::Vertex)};
        const int INDEX_BUFFER_SIZE{100*sizeof(decltype(Assets::Model::indices)::value_type)};
        const int VP_BUFFER_SIZE{sizeof(glm::mat4)};

        unsigned int processedFrame{0};

        glm::mat4 vpMatrix;
            
        vk::UniqueSampler sampler;

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
        vk::UniqueDescriptorSetLayout descriptorSetLayout;
        vk::UniqueDescriptorPool descriptorPool;

        std::vector<std::unique_ptr<SwapChainFrame>> frames;
		std::vector<const char*> validationLayers;
        std::vector<PipelineSync> pipelineSync;
        Textures textures;
        Texture depthImage;

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

        struct
        {
            Buffer vertex;
            Buffer index;
        } buffers;

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
        vk::Format getSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        vk::Format getDepthFormat();
        void createDepthImage();
        vk::UniqueCommandBuffer oneTimeCommandsStart();
        void oneTimeCommandsEnd(vk::CommandBuffer commandBuffer);
        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        void createPipelineSync();
        void allocateTextures();
        void setTexturesLayouts();
        Buffer createBuffer(unsigned int size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
        Image createImage(unsigned int width, unsigned int height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlagBits properties);
        vk::UniqueImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);
        vk::UniqueSampler createSampler();
        void copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset);
        void copyBufferToImage(vk::Buffer buffer, vk::Image image, unsigned int width, unsigned int height);
        void createBuffers();
        void updateUniforms(unsigned int imageID);
        void createDescriptorSetLayout();
        void createDescriptorPool();
        void createDescriptorSets();
		bool isDeviceOK(const vk::PhysicalDevice &potDevice);
        uint32_t getMemoryType(uint32_t typeFlags, vk::MemoryPropertyFlags properties);

};
