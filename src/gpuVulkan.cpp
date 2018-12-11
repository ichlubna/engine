#include <algorithm>
#include <string.h>
#include <iostream>
#include <fstream>
#include "gpuVulkan.h"

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
	windowPtr->getVulkanSurface(&instance.get(), &*surface);
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

void GpuVulkan::createGraphicsPipeline()
{
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

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo .setVertexBindingDescriptionCount(0)
                    .setPVertexBindingDescriptions(nullptr)
                    .setVertexAttributeDescriptionCount(0)
                    .setPVertexAttributeDescriptions(nullptr);

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
    layoutInfo  .setSetLayoutCount(0)
                .setPSetLayouts(nullptr)
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
        //if(frame.commandBuffer = device->allocateCommandBuffersUnique(allocInfo))
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
        frame->commandBuffer->draw(3, 1, 0, 0);
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

void GpuVulkan::render()
{
    device->waitForFences(1, &*pipelineSync.at(processedFrame).fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    unsigned int imageID;
    if(device->acquireNextImageKHR(*swapChain, std::numeric_limits<uint64_t>::max(), *pipelineSync.at(processedFrame).semaphores.imgReady, nullptr, &imageID) == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapChain();
        return;
    }
    
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
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createCommandBuffers();
    createPipelineSync();
}

GpuVulkan::~GpuVulkan()
{
}
