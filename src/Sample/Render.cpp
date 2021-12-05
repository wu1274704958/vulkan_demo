#include <sample/render.hpp>
#include <utils/frame_rate.h>
#include <comm_comp/renderpass.hpp>

namespace vkd{

SampleRender* SampleRender::self_instance = nullptr;

bool SampleRender::dispatchEvent(const evt::Event& e) {
	switch (e.type)
	{
		case evt::EventType::WindowReSize:
			auto d = e.GetEvent<evt::EventType::WindowReSize>();
			onWindowResize((uint32_t)d.w, (uint32_t)d.h);
			return true;
		break;
	}
	return scene_obj->dispatchEvent(e);
}

VkBool32 SampleRender::DebugReportCallbackEXT(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* pUserData) {
	auto ptr = (SampleRender*)pUserData;
	if (ptr == nullptr)
		return false;
	printf("Type = %d ObjId = %lld location = %zu code = %d\nLayerPrefix = %s\nMessage = %s", objectType, object, location, messageCode,
		pLayerPrefix, pMessage);
	return ptr->onDebugReportError(objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCallback, pAllocator);
	}
}

void SampleRender::init(int w, int h)
{
	if (enableValidationLayers)
	{
		std::vector<const char*> validationLayers;
		onFillValidationLayers(validationLayers);
		ValidationLayers = std::optional(validationLayers);
	}
	onFillDeviceNeedExtensions(DeviceNeedExtensions);
	initWindow(w, h);
	onCreate();
	createInstance();
	setUpDebugCallback();
	createSurface();
	queueFamilyIndices = pickPhysicalDevice();
	createLogicalDevice(queueFamilyIndices);
	createSwapChain();
	createSwapchainImageViews();
	createDepthStencilAttachment();
	createRenderPass();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSemaphores();
	createDrawFences();
	onInit();
	self_instance = this;
	initScene();
	scene_obj->init();
	isInit = true;
	engineState = EngineState::Initialized;
}

void SampleRender::initScene()
{
	scene_obj = std::make_shared<Object>("DefScene");
	scene = scene_obj->add_comp<Scene>();
	scene_obj->add_comp<DefRenderPass>();
}


void SampleRender::mainLoop()
{
	engineState = EngineState::Running;
	while (!glfwWindowShouldClose(window))
	{
		auto calc = gld::FrameRate::calculator();
		glfwPollEvents();
		if (!minWindowSize)
			drawFrame();
		onUpdate(lastFrameDelta);
		lastFrameDelta = gld::FrameRate::get_ms() ;
	}
	device.waitIdle();
	engineState = EngineState::Stoped;
}

void SampleRender::onUpdate(float delta)
{
	scene_obj->update(lastFrameDelta);
	scene_obj->late_update(lastFrameDelta);
}

void SampleRender::cleanUp()
{
	if (isInit){
		device.waitIdle();
		device.destroySemaphore(acquired_image_ready);
		device.destroySemaphore(render_complete);

		for (auto f : drawFences)
		{
			device.destroyFence(f);
		}

		cleanUpSwapChain();

		onCleanUp();

		device.destroyCommandPool(commandPool);
		device.destroy();
		vkDestroySurfaceKHR(instance, surface, nullptr);
		DestroyDebugReportCallbackEXT(instance, debugReport, nullptr);
		instance.destroy();
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	engineState = EngineState::Destroyed;
}

void SampleRender::onCleanUp()
{
	scene_obj->clean_up();
	scene.reset();
	scene_obj.reset();
	self_instance = nullptr;
}

void SampleRender::initWindow(uint32_t w, uint32_t h)
{

	width = w; height = h;

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	onCreateWindow();

	window = glfwCreateWindow(w, h, sample_name, nullptr, nullptr);

	eventConstructor.init(this,window);

	//glfwSetKeyCallback()
}

bool SampleRender::checkValidationLayerSupport()
{
	if (ValidationLayers)
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> props(count);
		vkEnumerateInstanceLayerProperties(&count, props.data());
		return wws::IsContain(props, ValidationLayers.value(), std::function([](const VkLayerProperties& a)->const char* {
			return a.layerName;
		}));
	}
	return true;
}

std::vector<const char*> SampleRender::getRequiredExtensions()
{
	uint32_t count;
	const char** requiredExtensions;
	requiredExtensions = glfwGetRequiredInstanceExtensions(&count);

	std::vector<const char*> extensions;
	extensions.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		extensions.push_back(requiredExtensions[i]);
	}
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	return extensions;
}
void SampleRender::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Enable ValidationLayer but not support!");
	}
	vk::ApplicationInfo app_info(sample_name, VK_MAKE_VERSION(0, 0, 1), "NoEngine", VK_MAKE_VERSION(0, 0, 1), VK_API_VERSION_1_2);

	const std::vector<const char*> extensions = getRequiredExtensions();
	auto enabledLayerNames = ValidationLayers ? vk::ArrayProxyNoTemporaries<const char* const>(ValidationLayers->size(), ValidationLayers->data()) : vk::ArrayProxyNoTemporaries<const char* const>();
	auto enabledExtensionNames = extensions.empty() ? vk::ArrayProxyNoTemporaries<const char* const>() : vk::ArrayProxyNoTemporaries(extensions.size(), extensions.data());

	vk::InstanceCreateInfo info(vk::InstanceCreateFlags(), &app_info, enabledLayerNames, enabledExtensionNames);

	instance = vk::createInstance(info);
	if (!instance)
	{
		throw std::runtime_error("Create instance failed!");
	}
}
void SampleRender::setUpDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugReportCallbackCreateInfoEXT callback_info = {};
	callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	callback_info.pfnCallback = &SampleRender::DebugReportCallbackEXT;
	callback_info.pUserData = this;
	if (CreateDebugReportCallbackEXT(instance, &callback_info, nullptr, &debugReport) != VK_SUCCESS)
	{
		throw std::runtime_error("create Debug Report Callback failed!");
	}

}
void SampleRender::createSurface()
{
	if (VK_SUCCESS != glfwCreateWindowSurface(instance, window, nullptr, &surface))
	{
		throw std::runtime_error("create surface failed!");
	}
}
QueueFamilyIndices SampleRender::pickPhysicalDevice()
{
	auto devices = instance.enumeratePhysicalDevices();
	std::vector<std::tuple<vk::PhysicalDevice, QueueFamilyIndices>> qualifiedDevices;
	for (int i = 0; i < devices.size(); ++i)
	{
		auto [v, indices] = isDeviceSuitable(devices[i]);
		if (v)
		{
			qualifiedDevices.push_back(std::make_tuple(devices[i], indices));
		}
	}
	if (qualifiedDevices.empty())
	{
		printf("failed to find a suitable GPU!\n");
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	auto [physicalDevice, indices] = onPickPhysicalDevice(qualifiedDevices);
	this->physicalDevice = physicalDevice;
	return indices;
}
std::tuple<bool, QueueFamilyIndices> SampleRender::isDeviceSuitable(const vk::PhysicalDevice& d)
{
	QueueFamilyIndices indices = findQueueFamilies(d);

	bool extensionSupport = checkDeviceExtensionSupport(d);
	bool swapChainAdequate = false;
	if (extensionSupport)
	{
		SwapChainSupportDetails details = querySwapChainSupport(d);
		swapChainAdequate = !(details.formats.empty() && details.presentMods.empty());
	}
	return std::make_tuple(indices.isComplete() && extensionSupport && swapChainAdequate, indices);
}

bool SampleRender::checkDeviceExtensionSupport(const vk::PhysicalDevice& d)
{
	auto extensionProps = d.enumerateDeviceExtensionProperties();
	return wws::IsContain(extensionProps, DeviceNeedExtensions, std::function([](const vk::ExtensionProperties& a) {
		return (const char*)a.extensionName;
	}));
}

QueueFamilyIndices SampleRender::findQueueFamilies(const vk::PhysicalDevice& d)
{
	QueueFamilyIndices indices;

	auto queueFamilyProp = d.getQueueFamilyProperties();

	for (int i = 0; i < queueFamilyProp.size(); ++i)
	{
		auto& it = queueFamilyProp[i];
		if (it.queueCount > 0 && it.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}
		VkBool32 supportPresent = false;
		if (it.queueCount > 0 && d.getSurfaceSupportKHR(i, surface))
		{
			indices.presentFamily = i;
		}
		if (indices.isComplete())
			break;
		++i;
	}
	return indices;
}
SwapChainSupportDetails SampleRender::querySwapChainSupport(const vk::PhysicalDevice& d)
{
	SwapChainSupportDetails res;
	res.capabilities = d.getSurfaceCapabilitiesKHR(surface);
	res.formats = d.getSurfaceFormatsKHR(surface);
	res.presentMods = d.getSurfacePresentModesKHR(surface);
	return res;
}

void SampleRender::createLogicalDevice(const QueueFamilyIndices& indices)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float priority = 1.0f;
	if (indices.graphicsFamily == indices.presentFamily)
	{
		queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), indices.graphicsFamily, 1, &priority));
	}
	else {
		queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), indices.graphicsFamily, 1, &priority));
		queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), indices.presentFamily, 1, &priority));
	}
	vk::PhysicalDeviceFeatures features;
	onSetPhysicalDeviceFeatures(features);
	auto enabledLayerNames = ValidationLayers ? vk::ArrayProxyNoTemporaries<const char* const>(ValidationLayers->size(), ValidationLayers->data()) : vk::ArrayProxyNoTemporaries<const char* const>();
	vk::DeviceCreateInfo info(vk::DeviceCreateFlags(), queueCreateInfos, enabledLayerNames, DeviceNeedExtensions, &features);
	device = physicalDevice.createDevice(info);
	if (!device) throw std::runtime_error("create Logic Device failed!");
	graphicsQueue = device.getQueue(indices.graphicsFamily, 0);
	presentQueue = device.getQueue(indices.presentFamily, 0);
}

void SampleRender::createSwapChain(bool recreate)
{
	auto format = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
	auto presentMode = chooseSwapPresent(physicalDevice.getSurfacePresentModesKHR(surface));
	auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	auto extent = chooseSwapExtent(surfaceCapabilities);
	const uint32_t temp[] = { queueFamilyIndices.graphicsFamily,queueFamilyIndices.presentFamily };
	vk::ArrayProxyNoTemporaries<const uint32_t> familyIndicesArr = queueFamilyIndices.isSame() ? vk::ArrayProxyNoTemporaries<const uint32_t>() :
		vk::ArrayProxyNoTemporaries(2, temp);
	vk::SwapchainKHR oldSwapchain = {};
	if (recreate && swapchain)
		oldSwapchain = swapchain;
	vk::SwapchainCreateInfoKHR info(vk::SwapchainCreateFlagsKHR(), surface, onSetSwapChainMinImageCount(surfaceCapabilities), format.format,
		format.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment,
		queueFamilyIndices.isSame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent, familyIndicesArr, surfaceCapabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, 1u, oldSwapchain
	);

	swapchain = device.createSwapchainKHR(info);
	if (!swapchain) throw std::runtime_error("create SwapChain failed!");

	swapchainImages = device.getSwapchainImagesKHR(swapchain);
	surfaceFormat = format.format;
	surfaceExtent = extent;
}
void SampleRender::createSwapchainImageViews()
{
	swapChainImageViews.clear();
	swapChainImageViews.resize(swapchainImages.size());

	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
		vk::ImageViewCreateInfo info(vk::ImageViewCreateFlags(), swapchainImages[i], vk::ImageViewType::e2D, surfaceFormat,
			vk::ComponentMapping(), { vk::ImageAspectFlagBits::eColor,0,1,0,1 });
		swapChainImageViews[i] = device.createImageView(info);
	}
}

 vk::SurfaceFormatKHR SampleRender::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
	if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
		return vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear);
	for (const auto& f : formats)
	{
		if (f.format == vk::Format::eR8G8B8A8Unorm && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return f;
	}
	return formats[0];
}
 vk::PresentModeKHR SampleRender::chooseSwapPresent(const std::vector<vk::PresentModeKHR>& presentMods)
{
	vk::PresentModeKHR best = vk::PresentModeKHR::eFifo;
	for (const auto& m : presentMods)
	{
		if (m == vk::PresentModeKHR::eMailbox)
			return m;
		else if (m == vk::PresentModeKHR::eImmediate)
			best = m;
	}
	return best;
}
 vk::Extent2D SampleRender::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else {
		vk::Extent2D actualExent = { width, height };
		actualExent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExent.width));
		actualExent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExent.height));

		return actualExent;
	}
}

 uint32_t SampleRender::onSetSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.minImageCount)
		imageCount = capabilities.maxImageCount;
	return imageCount;
}
 vk::Format SampleRender::onChooseDepthStencilFormat()
 {
	 return findSupportedFormat<vk::Format, vk::ImageTiling, vk::FormatFeatureFlagBits, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment,
		 vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm>();
 }

 uint32_t SampleRender::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlagBits properties) {
	 std::shared_ptr<vk::PhysicalDeviceMemoryProperties> memProperties = memPropCache;
	 for (uint32_t i = 0; i < memProperties->memoryTypeCount; i++)
	 {
		 if (typeFilter & (1 << i) && ((memProperties->memoryTypes[i].propertyFlags & properties) == properties))
		 {
			 return i;
		 }
	 }
	 throw std::runtime_error("failed to find memory type!");
 }

 void SampleRender::createDepthStencilAttachment()
 {
	 depthFormat = onChooseDepthStencilFormat();
	 vk::ImageCreateInfo imgInfo({}, vk::ImageType::e2D, depthFormat, vk::Extent3D(surfaceExtent.width, surfaceExtent.height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
		 vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, {});
	 depthAttachment.image = device.createImage(imgInfo);
	 depthAttachment.aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	 if (wws::eq_enum<vk::Format, vk::Format::eD16Unorm, vk::Format::eD32Sfloat>(depthFormat))
		 depthAttachment.aspect = vk::ImageAspectFlagBits::eDepth;
	 auto req = device.getImageMemoryRequirements(depthAttachment.image);

	 vk::MemoryAllocateInfo allocInfo(req.size, findMemoryType(req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
	 depthAttachment.mem = device.allocateMemory(allocInfo);

	 device.bindImageMemory(depthAttachment.image, depthAttachment.mem, 0);

	 vk::ImageViewCreateInfo viewInfo({}, depthAttachment.image, vk::ImageViewType::e2D, depthFormat, {},
		vk::ImageSubresourceRange(depthAttachment.aspect, 0, 1, 0, 1));
	 depthAttachment.view = device.createImageView(viewInfo);
 }

 void SampleRender::createRenderPass()
 {
	 auto depthImageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	 if (wws::eq_enum<vk::Format, vk::Format::eD16Unorm, vk::Format::eD32Sfloat>(depthFormat))
		 depthImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	 std::vector<vk::AttachmentDescription> attachment = {
		 vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),surfaceFormat,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,
		 vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR),
		 vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),depthFormat,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eDontCare,
		 vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,depthImageLayout)
	 };

	 vk::AttachmentReference attachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	 vk::AttachmentReference depthAttachmentRef(1, depthImageLayout);


	 std::array<vk::SubpassDescription, 1> subpassDesc = {};
	 subpassDesc[0] = vk::SubpassDescription();
	 subpassDesc[0].setColorAttachments(attachmentRef);
	 subpassDesc[0].setPDepthStencilAttachment(&depthAttachmentRef);
	 subpassDesc[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);


	 std::array<vk::SubpassDependency, 2> dependencies = {
		 vk::SubpassDependency(VK_SUBPASS_EXTERNAL,0,vk::PipelineStageFlagBits::eBottomOfPipe,vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::AccessFlagBits::eMemoryRead,
		 vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eColorAttachmentWrite,vk::DependencyFlagBits::eByRegion),
		 vk::SubpassDependency(0,VK_SUBPASS_EXTERNAL,vk::PipelineStageFlagBits::eColorAttachmentOutput,vk::PipelineStageFlagBits::eBottomOfPipe,
		 vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eColorAttachmentWrite,vk::AccessFlagBits::eMemoryRead,vk::DependencyFlagBits::eByRegion)
	 };

	 vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(), attachment, subpassDesc, dependencies);
	 renderPass = device.createRenderPass(info);
 }

 void SampleRender::createFramebuffers()
 {
	 std::vector<vk::ImageView> attachments = { swapChainImageViews[0],depthAttachment.view };
	 vk::FramebufferCreateInfo info({}, renderPass, attachments, surfaceExtent.width, surfaceExtent.height, 1);
	 framebuffers.clear();
	 framebuffers.reserve(swapChainImageViews.size());
	 for (int i = 0; i < swapChainImageViews.size(); ++i)
	 {
		 attachments[0] = swapChainImageViews[i];
		 framebuffers.push_back(device.createFramebuffer(info));
	 }
 }

 void SampleRender::createCommandPool()
 {
	 vk::CommandPoolCreateInfo info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, queueFamilyIndices.graphicsFamily);
	 commandPool = device.createCommandPool(info);
 }

 void SampleRender::createCommandBuffers()
 {
	 vk::CommandBufferAllocateInfo info(commandPool, vk::CommandBufferLevel::ePrimary, framebuffers.size());
	 commandbuffers = device.allocateCommandBuffers(info);
 }
 void SampleRender::createSemaphores()
 {
	 vk::SemaphoreCreateInfo info;
	 acquired_image_ready = device.createSemaphore(info);
	 render_complete = device.createSemaphore(info);
 }
 void SampleRender::createDrawFences()
 {
	 for (int i = 0; i < commandbuffers.size(); ++i)
	 {
		 drawFences.push_back(device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
	 }
 }

 void SampleRender::drawFrame()
 {
	 auto [res, imageIndex] = device.acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), acquired_image_ready);

	 if (res == vk::Result::eErrorOutOfDateKHR)
	 {
		 if (!minWindowSize)
			 recreateSwapChain();
	 }
	 else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
	 {
		 throw std::runtime_error("failed to present swap chain image!");
	 }
	 while (device.waitForFences(1, &drawFences[imageIndex], true, std::numeric_limits<uint64_t>::max()) == vk::Result::eTimeout) {}
	 auto cmdBuf = commandbuffers[imageIndex];
	 onDraw(cmdBuf, framebuffers[imageIndex]);
	 vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	 vk::SubmitInfo info(1, &acquired_image_ready, &waitStage, 1, &cmdBuf, 1, &render_complete);
	 vk::Fence drawFence = drawFences[imageIndex];
	 device.resetFences(1, &drawFence);
	 graphicsQueue.submit(info, drawFence);
	 vk::Result presentRes;
	 vk::PresentInfoKHR presentInfo(1, &render_complete, 1, &swapchain, &imageIndex, &presentRes);

	 presentQueue.presentKHR(presentInfo);
	 switch (presentRes)
	 {
	 case vk::Result::eSuccess:
		 break;
	 case vk::Result::eErrorOutOfDateKHR:
	 case vk::Result::eSuboptimalKHR:
		 if (!minWindowSize)recreateSwapChain();
		 break;
	 default:
		 throw std::runtime_error("failed to present swap chain image!");
		 break;
	 }
 }

 void SampleRender::onDraw(vk::CommandBuffer& cmd, vk::Framebuffer frameBuf)
 {
	 vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	 cmd.begin(beginInfo);
	 std::array<vk::ClearValue, 2> clearVals = { clearColorValue,vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0)) };
	 renderPassBeginInfo = vk::RenderPassBeginInfo(renderPass, currFrameBuffer = frameBuf, vk::Rect2D({ 0,0 }, surfaceExtent), clearVals);
	 //cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	 onRealDraw(cmd);

	 //cmd.endRenderPass();
	 cmd.end();
 }

void SampleRender::onRealDraw(vk::CommandBuffer& cmd)
{
	scene_obj->pre_draw(cmd);
	scene_obj->draw(cmd);
	scene_obj->after_draw(cmd);
}

 void SampleRender::recreateSwapChain()
 {
	 device.waitIdle();

	 cleanUpSwapChain();

	 createSwapChain();
	 createSwapchainImageViews();
	 createDepthStencilAttachment();
	 createRenderPass();
	 createFramebuffers();
	 createCommandBuffers();
	 onReCreateSwapChain();
 }

void SampleRender::onReCreateSwapChain()
{
	scene_obj->recreate_swapchain();
}


void SampleRender::cleanUpSwapChain()
{
	device.freeCommandBuffers(commandPool, commandbuffers);
	for (auto fb : framebuffers)
	{
	 device.destroyFramebuffer(fb);
	}

	onCleanUpPipeline();

	device.destroyRenderPass(renderPass);

	device.freeMemory(depthAttachment.mem);
	device.destroyImage(depthAttachment.image);
	device.destroyImageView(depthAttachment.view);

	for (int i = 0; i < swapChainImageViews.size(); ++i)
	{
	 device.destroyImageView(swapChainImageViews[i]);
	}
	device.destroySwapchainKHR(swapchain);
}

void SampleRender::onCleanUpPipeline()
{
	scene_obj->clean_up_pipeline();
}

void SampleRender::onCreate() {};
void SampleRender::onCreateWindow() {};
void SampleRender::onWindowResize(uint32_t w, uint32_t h)
 {
	 width = w; height = h;
	 minWindowSize = (w == 0 || h == 0);
	 if (minWindowSize)
		 return;
	 recreateSwapChain();
 }
void SampleRender::onFillValidationLayers(std::vector<const char*>& vec)
 {
	 vec.push_back("VK_LAYER_KHRONOS_validation");
 }
void SampleRender::onFillDeviceNeedExtensions(std::vector<const char*>& vec)
 {
	 vec.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
 }
void SampleRender::onSetPhysicalDeviceFeatures(const vk::PhysicalDeviceFeatures features) {}
VkBool32 SampleRender::onDebugReportError(
	 VkDebugReportObjectTypeEXT                  objectType,
	 uint64_t                                    object,
	 size_t                                      location,
	 int32_t                                     messageCode,
	 const char* pLayerPrefix,
	 const char* pMessage) {
	 return false;
 }
std::tuple<vk::PhysicalDevice, QueueFamilyIndices> SampleRender::onPickPhysicalDevice(const std::vector<std::tuple<vk::PhysicalDevice, QueueFamilyIndices>>& devices)
 {
	 return devices[0];
 }
SampleRender::~SampleRender()
{

}

}