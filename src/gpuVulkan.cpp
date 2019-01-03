#include <algorithm>
#include <string.h>
#include <iostream>
#include <fstream>
#include "gpuVulkan.h"

void GpuVulkan::updateViewProjectionMatrix(glm::mat4 vp)
{
    vpMatrix = vp;
}

void GpuVulkan::createInstance()
{
	vk::ApplicationInfo appInfo;
	appInfo	.setPApplicationName("Engine")
			.setApiVersion(VK_MAKE_VERSION(1,0,0))
			.setEngineVersion(VK_MAKE_VERSION(1,0,0))
			.setApplicationVersion(VK_MAKE_VERSION(1,0,0))
			.setPEngineName("I don't know");

	std::vector<const char*> extensions = {"VK_KHR_surface"};

	//validation layers
#ifndef NDEBUG
	validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	unsigned int layerCount;
	vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<vk::LayerProperties> availableLayers(layerCount);
	vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	bool enableValidation = true;
	for(const char* layer : validationLayers)
		if(!std::any_of(availableLayers.begin(), availableLayers.end(), 
			[layer](const vk::LayerProperties& avLayer) {return (strcmp(avLayer.layerName,  layer) == 0);}))
		{
			enableValidation=false;
			break;
		}
	if(!enableValidation)
		throw std::runtime_error("Validation layers not available in debug build.");
#endif

	windowPtr->addRequiredWindowExt(extensions);
	
	vk::InstanceCreateInfo createInfo;
	createInfo	.setPApplicationInfo(&appInfo)
				.setEnabledExtensionCount(extensions.size())
				.setPpEnabledExtensionNames(extensions.data())
				.setEnabledLayerCount(validationLayers.size())
				.setPpEnabledLayerNames(validationLayers.data());
    
    if(!(instance = vk::createInstanceUnique(createInfo)))
		throw std::runtime_error("Cannot create Vulkan instance.");

	//to check if needed extensions are supported
	/*unsigned int extensionCount = 0;
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<vk::ExtensionProperties> supportedExt(extensionCount);
	vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExt.data());*/

	//instance.createDebugReportCallbackEXT();
}

bool GpuVulkan::isDeviceOK(const vk::PhysicalDevice &potDevice)
{
		vk::PhysicalDeviceProperties properties = potDevice.getProperties();
		vk::PhysicalDeviceFeatures features = potDevice.getFeatures();

		if(properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && features.geometryShader)
		{
			unsigned int queueFamilyCount = 0;
			potDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);
			std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
			potDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());
			
			int i = 0;
			for(const auto& queueFamily : queueFamilies)
			{
				if(queueFamily.queueCount > 0)
				{
					if(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
						queueFamilyIDs.graphics = i;
					else if(queueFamily.queueFlags & vk::QueueFlagBits::eCompute)
						queueFamilyIDs.compute = i;
				}
				i++;
			}

			//might also check for extensions support for given device
			
			if(queueFamilyIDs.graphics != -1 && queueFamilyIDs.compute != -1)
			{
				return true;
			}
		}
		return false;
}

void GpuVulkan::selectPhysicalDevice()
{
	unsigned int deviceCount = 0;
	instance->enumeratePhysicalDevices(&deviceCount, nullptr);
	if(deviceCount == 0)
		throw std::runtime_error("No available Vulkan devices.");

	std::vector<vk::PhysicalDevice> devices(deviceCount);	
	instance->enumeratePhysicalDevices(&deviceCount, devices.data());
	bool chosen = false;
	for(const auto& potDevice : devices)
	{
		if(isDeviceOK(potDevice))
		{
			physicalDevice = potDevice;
			chosen = true;
			break;
		}
	}
	
	if(!chosen)
		throw std::runtime_error("No suitable device found.");		
}

void GpuVulkan::createDevice()
{	
	vk::DeviceQueueCreateInfo queueCreateInfos[3];

	queueCreateInfos[0].queueFamilyIndex = queueFamilyIDs.graphics;
	queueCreateInfos[0].queueCount = 1;
	float graphicsQueuePriority = 1.0f;
	queueCreateInfos[0].pQueuePriorities = &graphicsQueuePriority;

	queueCreateInfos[1].queueFamilyIndex = queueFamilyIDs.compute;
	queueCreateInfos[1].queueCount = 1;
	float computeQueuePriority = 1.0f;
	queueCreateInfos[1].pQueuePriorities = &computeQueuePriority;
	
	vk::PhysicalDeviceFeatures deviceFeatures = {};

	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo createInfo;
	createInfo	.setPQueueCreateInfos(queueCreateInfos)
				.setQueueCreateInfoCount(2)
				.setPEnabledFeatures(&deviceFeatures)
				.setEnabledLayerCount(validationLayers.size())
				.setPpEnabledLayerNames(validationLayers.data())
                .setEnabledExtensionCount(deviceExtensions.size())
                .setPpEnabledExtensionNames(deviceExtensions.data()); 
	if(!(device = physicalDevice.createDeviceUnique(createInfo, nullptr)))
		throw std::runtime_error("Cannot create a logical device.");

	queues.graphics = device->getQueue(queueFamilyIDs.graphics, 0);
	queues.compute = device->getQueue(queueFamilyIDs.compute, 0);
}

void GpuVulkan::createSurface()
{
    vk::SurfaceKHR tmpSurface;
	windowPtr->getVulkanSurface(&instance.get(), &tmpSurface);
    
    vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderStatic> surfaceDeleter(*instance);
    surface = vk::UniqueSurfaceKHR(tmpSurface, surfaceDeleter);  

	if(!physicalDevice.getSurfaceSupportKHR(queueFamilyIDs.graphics, *surface))
		throw std::runtime_error("Chosen graphics queue doesn't support presentation.");

    queues.present = queues.graphics;
    queueFamilyIDs.present = queueFamilyIDs.graphics;
}

void GpuVulkan::createSwapChain()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	physicalDevice.getSurfaceCapabilitiesKHR(*surface, &surfaceCapabilities);

	unsigned int formatCount;
	physicalDevice.getSurfaceFormatsKHR(*surface, &formatCount, nullptr);
	std::vector<vk::SurfaceFormatKHR> formats(formatCount);
	physicalDevice.getSurfaceFormatsKHR(*surface, &formatCount, formats.data());
	
	unsigned int pmCount;
	physicalDevice.getSurfacePresentModesKHR(*surface, &pmCount, nullptr);
	std::vector<vk::PresentModeKHR> presentModes(pmCount);
	physicalDevice.getSurfacePresentModesKHR(*surface, &pmCount, presentModes.data());

	if(formats.empty() || presentModes.empty())
		throw std::runtime_error("Insufficient swap chain available properties.");

	vk::SurfaceFormatKHR format{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
    if(formats.size() != 1  && formats[0].format != vk::Format::eUndefined)
	{
		bool notFound = true;
		for(const auto& potFormat : formats)
			if(potFormat.colorSpace == format.colorSpace && potFormat.format == format.format)
			{
				notFound = false;
				break;
			}
		if(notFound)
			throw std::runtime_error("No suitable surface format found.");
	}

    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    for(const auto& potPm : presentModes)
		if(potPm == vk::PresentModeKHR::eMailbox)
		{	
			presentMode = potPm;
			break;
		}
		else if(potPm == vk::PresentModeKHR::eImmediate)
			presentMode = potPm;

	//auto winSize = windowPtr->getSize();
	auto winSize = windowPtr->getFramebufferSize();
    //might differ TODO
	extent = vk::Extent2D(winSize.width, winSize.height);
	//extent = vk::Extent2D(1920,1200);

	unsigned int imageCount = surfaceCapabilities.minImageCount + 1; 
	if(surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount )
		imageCount = surfaceCapabilities.maxImageCount;

    swapChainImgFormat = format.format;	
    vk::SwapchainCreateInfoKHR createInfo;
	createInfo	.setSurface(*surface)
				.setMinImageCount(imageCount)
				.setImageFormat(swapChainImgFormat)
				.setImageColorSpace(format.colorSpace)
				.setImageExtent(extent)
				.setImageArrayLayers(1)
				.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
				.setPreTransform(surfaceCapabilities.currentTransform)
				.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
				.setPresentMode(presentMode)
                .setOldSwapchain(vk::SwapchainKHR())
				.setClipped(VK_TRUE);
    
    unsigned int indices[2] = {static_cast<unsigned int>(queueFamilyIDs.graphics), static_cast<unsigned int>(queueFamilyIDs.present)};
	if(queueFamilyIDs.graphics != queueFamilyIDs.present)
		createInfo	.setImageSharingMode(vk::SharingMode::eConcurrent)
					.setQueueFamilyIndexCount(2)
					.setPQueueFamilyIndices(indices);
	else
		createInfo	.setImageSharingMode(vk::SharingMode::eExclusive);

    if(!(swapChain = device->createSwapchainKHRUnique(createInfo, nullptr)))
		throw std::runtime_error("Failed to create swap chain.");
    device->getSwapchainImagesKHR(*swapChain, &imageCount, nullptr);
    std::vector<vk::Image> swapChainImages(imageCount);

	device->getSwapchainImagesKHR(*swapChain, &imageCount, swapChainImages.data());
    for(auto image : swapChainImages)
    {
        frames.push_back(std::make_unique<SwapChainFrame>());
        frames.back()->image = image;
    }
}

void GpuVulkan::createSwapChainImageViews()
{
    for(auto &frame : frames)
    {
        vk::ImageViewCreateInfo createInfo;
        createInfo  .setImage(frame->image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(swapChainImgFormat)
                    .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity))
                    .setSubresourceRange(vk::ImageSubresourceRange().setAspectMask(vk::ImageAspectFlagBits::eColor)
                                                                    .setBaseMipLevel(0)
                                                                    .setLevelCount(1)
                                                                    .setBaseArrayLayer(0)
                                                                    .setLayerCount(1));

        if(!(frame->imageView = device->createImageViewUnique(createInfo, nullptr)))
            throw std::runtime_error("Cannot create a swap chain image view.");
    }
}

std::vector<char> GpuVulkan::loadShader(const char *path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if(!file)
       throw std::runtime_error("Cannot open shader file.");

    size_t size = file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
     
    return buffer;
}

vk::UniqueShaderModule GpuVulkan::createShaderModule(std::vector<char> source)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo  .setCodeSize(source.size())
                .setPCode(reinterpret_cast<const uint32_t*>(source.data()));
    vk::UniqueShaderModule module;
    if(!(module = device->createShaderModuleUnique(createInfo)))
        throw std::runtime_error("Cannot create a shader module.");
    return module;
}  

void GpuVulkan::createRenderPass()
{
    vk::AttachmentDescription colorAttachement;
    colorAttachement.setFormat(swapChainImgFormat)
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                    .setInitialLayout(vk::ImageLayout::eUndefined)
                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachementRef;
    colorAttachementRef .setAttachment(0)
                        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachementRef);
    
    vk::SubpassDependency dependency;
    dependency  .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput) 
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo createInfo;
    createInfo  .setAttachmentCount(1)
                .setPAttachments(&colorAttachement)
                .setSubpassCount(1)
                .setPSubpasses(&subpass)
                .setDependencyCount(1)
                .setPDependencies(&dependency);

    if(!(renderPass = device->createRenderPassUnique(createInfo)))
        throw std::runtime_error("Cannot create render pass.");
}       

void GpuVulkan::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding   .setBinding(bindings.viewProjectionMatrix)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo createInfo;
    createInfo  .setBindingCount(1)
                .setPBindings(&layoutBinding);

    if(!(descriptorSetLayout = device->createDescriptorSetLayoutUnique(createInfo)))
        throw std::runtime_error("Cannot create descriptor set layout.");
}

void GpuVulkan::createGraphicsPipeline()
{
    //TODO split into smaller ones maybe?
    auto vertexShader = loadShader("../precompiled/vertex.spv"); 
    auto fragmentShader = loadShader("../precompiled/fragment.spv");
    vk::UniqueShaderModule vertexModule = createShaderModule(vertexShader); 
    vk::UniqueShaderModule fragmentModule = createShaderModule(fragmentShader);

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(2);
    shaderStages.at(0)  .setStage(vk::ShaderStageFlagBits::eVertex)
                        .setModule(*vertexModule)
                        .setPName("main")
                        .setPSpecializationInfo(nullptr); //can set shader constants - changing behaviour at creation
    shaderStages.at(1)  .setStage(vk::ShaderStageFlagBits::eFragment)
                        .setModule(*fragmentModule)
                        .setPName("main");

    vk::VertexInputBindingDescription binding;
    binding .setBinding(0)
            .setStride(sizeof(Assets::Vertex))
            .setInputRate(vk::VertexInputRate::eVertex);

    std::vector<vk::VertexInputAttributeDescription> attributes;
    attributes.push_back({0, locations.position, vk::Format::eR32G32B32Sfloat, offsetof(Assets::Vertex, position)});

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo .setVertexBindingDescriptionCount(1)
                    .setPVertexBindingDescriptions(&binding)
                    .setVertexAttributeDescriptionCount(attributes.size())
                    .setPVertexAttributeDescriptions(attributes.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo   .setTopology(vk::PrimitiveTopology::eTriangleList)
                        .setPrimitiveRestartEnable(false);

    vk::Viewport viewport;
    viewport.setX(0.0f)
            .setY(0.0f)
            .setWidth(extent.width)
            .setHeight(extent.height)
            .setMinDepth(0.0f)
            .setMaxDepth(1.0f);

    vk::Rect2D scissor;
    scissor .setOffset(vk::Offset2D(0,0))
            .setExtent(extent);

    vk::PipelineViewportStateCreateInfo viewportStateInfo;
    viewportStateInfo   .setViewportCount(1)
                        .setPViewports(&viewport)
                        .setScissorCount(1)
                        .setPScissors(&scissor);

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
    rasterizerInfo  .setDepthClampEnable(false)
                    .setRasterizerDiscardEnable(false)
                    .setPolygonMode(vk::PolygonMode::eFill)
                    .setLineWidth(1.0f)
                    .setCullMode(vk::CullModeFlagBits::eBack)
                    .setFrontFace(vk::FrontFace::eClockwise)
                    .setDepthBiasEnable(false);

    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo .setSampleShadingEnable(false)
                    .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                    .setMinSampleShading(1.0f)
                    .setPSampleMask(nullptr)
                    .setAlphaToCoverageEnable(false)
                    .setAlphaToOneEnable(false);

    vk::PipelineColorBlendAttachmentState colorBlendAttachement;
    colorBlendAttachement   .setColorWriteMask( vk::ColorComponentFlagBits::eR |
                                                vk::ColorComponentFlagBits::eG |
                                                vk::ColorComponentFlagBits::eB |
                                                vk::ColorComponentFlagBits::eA)
                            .setBlendEnable(false)
                            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                            .setColorBlendOp(vk::BlendOp::eAdd)
                            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                            .setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo blendStateInfo;
    blendStateInfo  .setLogicOpEnable(false)
                    .setLogicOp(vk::LogicOp::eCopy)
                    .setAttachmentCount(1)
                    .setPAttachments(&colorBlendAttachement)
                    .setBlendConstants({0.0f,0.0f,0.0f,0.0f});

    vk::PipelineLayoutCreateInfo layoutInfo;
    layoutInfo  .setSetLayoutCount(1)
                .setPSetLayouts(&*descriptorSetLayout)
                .setPushConstantRangeCount(0)
                .setPPushConstantRanges(nullptr);

    if(!(pipelineLayout = device->createPipelineLayoutUnique(layoutInfo)))
        throw std::runtime_error("Cannot create pipeline layout.");

    vk::GraphicsPipelineCreateInfo createInfo;
    createInfo  .setStageCount(shaderStages.size())
                .setPStages(shaderStages.data())
                .setPVertexInputState(&vertexInputInfo)
                .setPInputAssemblyState(&inputAssemblyInfo)
                .setPViewportState(&viewportStateInfo)
                .setPRasterizationState(&rasterizerInfo)
                .setPMultisampleState(&multisampleInfo)
                .setPDepthStencilState(nullptr)
                .setPColorBlendState(&blendStateInfo)
                .setPDynamicState(nullptr)
                .setLayout(*pipelineLayout)
                .setRenderPass(*renderPass)
                .setSubpass(0)
                .setBasePipelineHandle(nullptr)
                .setBasePipelineIndex(-1);
    
    if(!(graphicsPipeline = (device->createGraphicsPipelineUnique(nullptr, createInfo))))
        throw std::runtime_error("Cannot create graphics pipeline.");
}

void GpuVulkan::createFramebuffers()
{
    for(auto &frame : frames)
    {
        std::vector<vk::ImageView> attachments;
        attachments.push_back(*frame->imageView);
    
        vk::FramebufferCreateInfo createInfo;
        createInfo  .setRenderPass(*renderPass)
                    .setAttachmentCount(attachments.size())
                    .setPAttachments(attachments.data())
                    .setWidth(extent.width)
                    .setHeight(extent.height)
                    .setLayers(1);

        if(!(frame->frameBuffer = device->createFramebufferUnique(createInfo, nullptr)))
            throw std::runtime_error("Cannot create frame buffer.");
    }
}

void GpuVulkan::createCommandPool()
{
    vk::CommandPoolCreateInfo createInfo;
    createInfo  .setQueueFamilyIndex(queueFamilyIDs.graphics);

    if(!(commandPool = device->createCommandPoolUnique(createInfo, nullptr)))
        throw std::runtime_error("Cannot create command pool.");
}

void GpuVulkan::createCommandBuffers()
{
    for(auto &frame : frames)
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo   .setCommandPool(*commandPool)
                    .setLevel(vk::CommandBufferLevel::ePrimary)
                    .setCommandBufferCount(1);
        if(!(frame->commandBuffer = std::move(device->allocateCommandBuffersUnique(allocInfo).front())))
            throw std::runtime_error("Failed to allocate command buffers.");
        vk::CommandBufferBeginInfo bufferBeginInfo;
        bufferBeginInfo .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
                        .setPInheritanceInfo(nullptr);

        if(frame->commandBuffer->begin(&bufferBeginInfo) != vk::Result::eSuccess)
            throw std::runtime_error("Command buffer recording couldn't begin.");
    
        vk::RenderPassBeginInfo passBeginInfo;
        vk::ClearValue clearValue(vk::ClearColorValue().setFloat32({0.0,0.0,0.0,1.0}));
        passBeginInfo   .setRenderPass(*renderPass)
                        .setFramebuffer(*frame->frameBuffer)
                        .setRenderArea(vk::Rect2D(vk::Offset2D(), extent))
                        .setClearValueCount(1)
                        .setPClearValues(&clearValue);

        frame->commandBuffer->beginRenderPass(passBeginInfo, vk::SubpassContents::eInline);
        frame->commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
        vk::DeviceSize offsets[] = {0};
        frame->commandBuffer->bindVertexBuffers(0, 1, &*buffers.vertex.buffer, offsets);
        frame->commandBuffer->bindIndexBuffer(*buffers.index.buffer, 0, vk::IndexType::eUint16);
        frame->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, &*frame->descriptorSet, 0, nullptr);
        frame->commandBuffer->drawIndexed(10, 1, 0, 0, 0);
        frame->commandBuffer->endRenderPass();

        frame->commandBuffer->end();
        /* != vk::Result::eSuccess)
            throw std::runtime_error("Cannot record command buffer.");*/    
    }
}

void GpuVulkan::createPipelineSync()
{
    pipelineSync.resize(CONCURRENT_FRAMES_COUNT);

    for(auto &sync : pipelineSync)
    {
        vk::SemaphoreCreateInfo semCreateInfo;
        vk::FenceCreateInfo fenCreateInfo;
        fenCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        if( !(sync.semaphores.imgReady = device->createSemaphoreUnique(semCreateInfo)) ||
            !(sync.semaphores.renderReady = device->createSemaphoreUnique(semCreateInfo)) ||
            !(sync.fence = device->createFenceUnique(fenCreateInfo)))
            throw std::runtime_error("Cannot create pipeline synchronization.");
    }
}

uint32_t GpuVulkan::getMemoryType(uint32_t typeFlag, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memoryProps;
    memoryProps = physicalDevice.getMemoryProperties();
    
    for(unsigned int i=0; i<memoryProps.memoryTypeCount; i++)
        if((typeFlag & (1<<i)) && ((memoryProps.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    throw std::runtime_error("Necessary memory type not available.");
}

GpuVulkan::Buffer GpuVulkan::createBuffer(unsigned int size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    Buffer buffer;
    vk::BufferCreateInfo createInfo;
    createInfo  .setSize(size)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);

    if(!(buffer.buffer = device->createBufferUnique(createInfo)))
        throw std::runtime_error("Cannot create vertex buffer!");

    vk::MemoryRequirements requirements = device->getBufferMemoryRequirements(*buffer.buffer);

    vk::MemoryAllocateInfo allocateInfo;
    allocateInfo.setAllocationSize(requirements.size)
                .setMemoryTypeIndex(getMemoryType(  requirements.memoryTypeBits,
                                                    properties));
    
    if(!(buffer.memory = device->allocateMemoryUnique(allocateInfo)))
        throw std::runtime_error("Cannot allocate vertex buffer memory.");
    
    device->bindBufferMemory(*buffer.buffer, *buffer.memory, 0);
   
    return buffer;
}

void GpuVulkan::createBuffers()
{
    buffers.vertex = createBuffer(VERTEX_BUFFER_SIZE, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
    buffers.index = createBuffer(INDEX_BUFFER_SIZE, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

    for(auto &frame : frames)
    {
        frame->uniformVpMatrix = createBuffer(VP_BUFFER_SIZE, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }
}

void GpuVulkan::updateUniforms(unsigned int imageID)
{
    void *data;
    device->mapMemory(*(frames[imageID]->uniformVpMatrix.memory), 0, VP_BUFFER_SIZE, vk::MemoryMapFlags(), &data);
    memcpy(data, &vpMatrix, VP_BUFFER_SIZE);
    device->unmapMemory(*(frames[imageID]->uniformVpMatrix.memory)); 
     
}

void GpuVulkan::createDescriptorPool()
{
    vk::DescriptorPoolSize poolSize;
    poolSize.setDescriptorCount(frames.size())
            .setType(vk::DescriptorType::eUniformBuffer);  

    vk::DescriptorPoolCreateInfo createInfo;
    createInfo  .setPoolSizeCount(1)
                .setMaxSets(frames.size())
                .setPPoolSizes(&poolSize);
    if(!(descriptorPool = device->createDescriptorPoolUnique(createInfo)))
        throw std::runtime_error("Cannot create a descriptor pool.");
}

void GpuVulkan::createDescriptorSets()
{
    for(auto &frame : frames)
    {
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo   .setDescriptorPool(*descriptorPool)
                    .setDescriptorSetCount(1)
                    .setPSetLayouts(&*descriptorSetLayout);
        
        frame->descriptorSet = std::move(device->allocateDescriptorSetsUnique(allocInfo).front());
        
        vk::DescriptorBufferInfo bufferInfo;
        bufferInfo  .setBuffer(*frame->uniformVpMatrix.buffer)
                    .setOffset(0)
                    .setRange(VP_BUFFER_SIZE);

        vk::WriteDescriptorSet writeSet;
        writeSet.setDstSet(*frame->descriptorSet)
                .setDstBinding(bindings.viewProjectionMatrix)
                .setDstArrayElement(0)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setPBufferInfo(&bufferInfo);

       device->updateDescriptorSets(1, &writeSet, 0, nullptr);
            
    }
}

void GpuVulkan::copyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset)
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo   .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandPool(*commandPool)
                .setCommandBufferCount(1);

    vk::UniqueCommandBuffer commandBuffer = std::move(device->allocateCommandBuffersUnique(allocInfo).front());
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    
    commandBuffer->begin(beginInfo);
    vk::BufferCopy copyPart;
    copyPart.setSrcOffset(srcOffset)
            .setDstOffset(dstOffset)
            .setSize(size);
    commandBuffer->copyBuffer(src, dst, 1, &copyPart);
    commandBuffer->end();

    vk::SubmitInfo submitInfo;
    submitInfo  .setCommandBufferCount(1)
                .setPCommandBuffers(&*commandBuffer);

    queues.graphics.submit(1, &submitInfo, vk::Fence());
    queues.graphics.waitIdle();   
}

void GpuVulkan::addModel(std::shared_ptr<Assets::Model> model)
{
    unsigned int size = model->vertices.size()*sizeof(Assets::Vertex);

    Buffer staging = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void *memoryPtr;
    device->mapMemory(*staging.memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags(), &memoryPtr);
    memcpy(memoryPtr, model->vertices.data(), size);
    device->unmapMemory(*staging.memory);
    
    copyBuffer(*staging.buffer, *buffers.vertex.buffer, size, 0, buffers.vertex.top);
    buffers.vertex.top += size;

    size = model->indices.size()*sizeof(decltype(model->indices)::value_type);
    device->mapMemory(*staging.memory, 0, VK_WHOLE_SIZE, vk::MemoryMapFlags(), &memoryPtr);
    memcpy(memoryPtr, model->indices.data(), size);
    device->unmapMemory(*staging.memory);
    
    copyBuffer(*staging.buffer, *buffers.index.buffer, size, 0, buffers.index.top);
    buffers.index.top += size;
}

void GpuVulkan::render()
{
    device->waitForFences(1, &*pipelineSync.at(processedFrame).fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    unsigned int imageID;
    if(device->acquireNextImageKHR(*swapChain, std::numeric_limits<uint64_t>::max(), *pipelineSync.at(processedFrame).semaphores.imgReady, nullptr, &imageID) == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapChain();
        return;
    }
    
    updateUniforms(imageID);

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo;
    submitInfo  .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&*pipelineSync.at(processedFrame).semaphores.imgReady)
                .setPWaitDstStageMask(waitStages)
                .setCommandBufferCount(1)
                .setPCommandBuffers(&*frames[imageID]->commandBuffer)
                .setSignalSemaphoreCount(1)
                .setPSignalSemaphores(&*pipelineSync.at(processedFrame).semaphores.renderReady);

    device->resetFences(1, &*pipelineSync.at(processedFrame).fence);

    if(queues.graphics.submit(1, &submitInfo, *pipelineSync.at(processedFrame).fence) != vk::Result::eSuccess)
        throw std::runtime_error("Cannot submit draw command buffer.");

    vk::PresentInfoKHR presentInfo;
    presentInfo .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&*pipelineSync.at(processedFrame).semaphores.renderReady)
                .setSwapchainCount(1)
                .setPSwapchains(&*swapChain)
                .setPImageIndices(&imageID);

    queues.present.presentKHR(&presentInfo);

    processedFrame = (processedFrame+1) % CONCURRENT_FRAMES_COUNT;
}

void GpuVulkan::recreateSwapChain()
{
    /* HANDLING MINIMIZATION BY STOPPING, MAYBE NOT NECESSARY
     int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    */

    device->waitIdle();

    frames.clear();
    graphicsPipeline.reset();
    pipelineLayout.reset();
    renderPass.reset();
    swapChain.reset();
    
	createSwapChain();
    createSwapChainImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandBuffers();
}

GpuVulkan::GpuVulkan(Window* w) : Gpu(w)
{
	createInstance();
	selectPhysicalDevice();
	createDevice();
	createSurface();
	createSwapChain();
    createSwapChainImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandPool();
    createCommandBuffers();
    createPipelineSync();
}

GpuVulkan::~GpuVulkan()
{
}
