#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <common.hpp>
#include <optional>
#include <json.hpp>
namespace vkd {

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback, const VkAllocationCallbacks* pAllocator);

	struct QueueFamilyIndices {
		int graphicsFamily = -1;
		int presentFamily = -1;
		bool isComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
		bool isSame()
		{
			return isComplete() && graphicsFamily == presentFamily;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentMods;
		SwapChainSupportDetails() noexcept(true) = default;
		SwapChainSupportDetails(const SwapChainSupportDetails&) = default;
		SwapChainSupportDetails(SwapChainSupportDetails&& other) noexcept
		{
			capabilities = other.capabilities;
			formats = std::move(other.formats);
			presentMods = std::move(other.presentMods);
		}
	};

	class SampleRender {
	public:
		SampleRender() 
			: memPropCache(std::function([](std::tuple<vk::PhysicalDevice&>& args){
				return std::get<0>(args).getMemoryProperties();
			}),physicalDevice),
			clearColorValue(clearColorArr) {
				clearColorValue.depthStencil.depth = 1.0f;
			}
		SampleRender(bool enableValidationLayers,const char* sample_name) 
			: memPropCache(std::function([](std::tuple<vk::PhysicalDevice&>& args) {
				return std::get<0>(args).getMemoryProperties();
			}), physicalDevice),
			clearColorValue(clearColorArr)
		{
			clearColorValue.depthStencil.depth = 1.0f;

			this->enableValidationLayers = enableValidationLayers;
			this->sample_name = sample_name;
		}
		SampleRender(const SampleRender&) = delete;
		SampleRender(SampleRender&&) = delete;
		~SampleRender()
		{
			DestroyDebugReportCallbackEXT(instance,debugReport,nullptr);	
		}
		
		void init(int w,int h)
		{
			if(enableValidationLayers)
			{
				std::vector<const char*> validationLayers;
				onFillValidationLayers(validationLayers);
				ValidationLayers = std::optional(validationLayers);
			}
			onFillDeviceNeedExtensions(DeviceNeedExtensions);
			initWindow(w,h);
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
			onInit();
		}
		virtual void mainLoop() {
			while (!glfwWindowShouldClose(window))
			{
				glfwPollEvents();
				onUpdate(lastFrameDelta);
				if (!minWindowSize)
					drawFrame();
			}
			device.waitIdle();
		}
	protected:
		void initWindow(uint32_t w, uint32_t h) {
			
			width = w;height = h;

			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			onCreateWindow();

			window = glfwCreateWindow(w, h, sample_name, nullptr, nullptr);

			glfwSetWindowUserPointer(window, this);

			glfwSetWindowSizeCallback(window, WindowReSize);
		}
		bool checkValidationLayerSupport() {
			if (ValidationLayers)
			{
				uint32_t count;
				vkEnumerateInstanceLayerProperties(&count, nullptr);
				std::vector<VkLayerProperties> props(count);
				vkEnumerateInstanceLayerProperties(&count, props.data());
				return IsContain(props,ValidationLayers.value(),std::function([](const VkLayerProperties& a)->const char*{
					return a.layerName;
				}));
			}
			return true;
		}
		std::vector<const char*> getRequiredExtensions()
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
		void createInstance()
		{
			if (enableValidationLayers && !checkValidationLayerSupport())
			{
				throw std::runtime_error("Enable ValidationLayer but not support!");
			}
			vk::ApplicationInfo app_info(sample_name,VK_MAKE_VERSION(0,0,1),"NoEngine", VK_MAKE_VERSION(0, 0, 1),VK_API_VERSION_1_2);

			const std::vector<const char*> extensions = getRequiredExtensions();
			auto enabledLayerNames = ValidationLayers ? vk::ArrayProxyNoTemporaries<const char* const>(ValidationLayers->size(),ValidationLayers->data()) : vk::ArrayProxyNoTemporaries<const char* const>();
			auto enabledExtensionNames = extensions.empty() ? vk::ArrayProxyNoTemporaries<const char*const>() : vk::ArrayProxyNoTemporaries(extensions.size(),extensions.data());

			vk::InstanceCreateInfo info(vk::InstanceCreateFlags(),&app_info,enabledLayerNames,enabledExtensionNames); 

			instance = vk::createInstance(info);
			if (!instance)
			{
				throw std::runtime_error("Create instance failed!");
			}
		}
		void setUpDebugCallback()
		{
			if (!enableValidationLayers)
				return;

			VkDebugReportCallbackCreateInfoEXT callback_info = {};
			callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT  ;
			callback_info.pfnCallback = &SampleRender::DebugReportCallbackEXT;
			callback_info.pUserData = this;
			if (CreateDebugReportCallbackEXT(instance, &callback_info, nullptr, &debugReport) != VK_SUCCESS)
			{
				throw std::runtime_error("create Debug Report Callback failed!");
			}

		}
		void createSurface()
		{
			if (VK_SUCCESS != glfwCreateWindowSurface(instance, window, nullptr, &surface))
			{
				throw std::runtime_error("create surface failed!");
			}
		}

		QueueFamilyIndices pickPhysicalDevice()
		{
			auto devices = instance.enumeratePhysicalDevices();
			std::vector<std::tuple<vk::PhysicalDevice,QueueFamilyIndices>> qualifiedDevices;
			for (int i = 0; i < devices.size(); ++i)
			{
				auto[v,indices] = isDeviceSuitable(devices[i]);
				if (v)
				{	
					qualifiedDevices.push_back(std::make_tuple(devices[i],indices));
				}
			}
			if (qualifiedDevices.empty())
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}
			physicalDevice = onPickPhysicalDevice(qualifiedDevices);
		}

		std::tuple<bool,QueueFamilyIndices> isDeviceSuitable(const vk::PhysicalDevice& d)
		{	
			QueueFamilyIndices indices = findQueueFamilies(d);

			bool extensionSupport = checkDeviceExtensionSupport(d);
			bool swapChainAdequate = false;
			if (extensionSupport)
			{
				SwapChainSupportDetails details = querySwapChainSupport(d);
				swapChainAdequate = !(details.formats.empty() && details.presentMods.empty());
			}
			return std::make_tuple(indices.isComplete() && extensionSupport && swapChainAdequate,indices);
		}

		bool checkDeviceExtensionSupport(const vk::PhysicalDevice& d)
		{
			auto extensionProps = d.enumerateDeviceExtensionProperties();
			return IsContain(extensionProps,DeviceNeedExtensions,std::function([](const vk::ExtensionProperties& a){
				return (const char*)a.extensionName;
			}));
		}

		QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& d)
		{
			QueueFamilyIndices indices;

			auto queueFamilyProp = d.getQueueFamilyProperties();

			for (int i = 0;i < queueFamilyProp.size();++i)
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
		SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& d)
		{
			SwapChainSupportDetails res;
			res.capabilities = d.getSurfaceCapabilitiesKHR(surface);
			res.formats = d.getSurfaceFormatsKHR(surface);
			res.presentMods = d.getSurfacePresentModesKHR(surface);
			return res;
		}

		void createLogicalDevice(QueueFamilyIndices indices)
		{
			std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
			float priority = 1.0f;
			if (indices.graphicsFamily == indices.presentFamily)
			{
				queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(),indices.graphicsFamily,1,&priority));
			}else{
				queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), indices.graphicsFamily, 1, &priority));
				queueCreateInfos.push_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), indices.presentFamily, 1, &priority));
			}
			vk::PhysicalDeviceFeatures features;
			onSetPhysicalDeviceFeatures(features);
			auto enabledLayerNames = ValidationLayers ? vk::ArrayProxyNoTemporaries<const char* const>(ValidationLayers->size(), ValidationLayers->data()) : vk::ArrayProxyNoTemporaries<const char* const>();
			vk::DeviceCreateInfo info(vk::DeviceCreateFlags(),queueCreateInfos,enabledLayerNames,DeviceNeedExtensions,&features);
			device = physicalDevice.createDevice(info);
			if(!device) throw std::runtime_error("create Logic Device failed!");
			graphicsQueue = device.getQueue(indices.graphicsFamily,0);
			presentQueue = device.getQueue(indices.presentFamily, 0);
		}

		void createSwapChain(bool recreate = false)
		{
			auto format = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
			auto presentMode = chooseSwapPresent(physicalDevice.getSurfacePresentModesKHR(surface));
			auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
			auto extent = chooseSwapExtent(surfaceCapabilities);
			const uint32_t temp [] = {queueFamilyIndices.graphicsFamily,queueFamilyIndices.presentFamily};
			vk::ArrayProxyNoTemporaries<const uint32_t> familyIndicesArr = queueFamilyIndices.isSame() ? vk::ArrayProxyNoTemporaries(temp[0]) :
				vk::ArrayProxyNoTemporaries(2,temp);
			vk::SwapchainKHR oldSwapchain = {};
			if(recreate && swapchain)
				oldSwapchain = swapchain;
			vk::SwapchainCreateInfoKHR info(vk::SwapchainCreateFlagsKHR(),surface,onSetSwapChainMinImageCount(surfaceCapabilities),format.format,
				format.colorSpace, extent,1,vk::ImageUsageFlagBits::eColorAttachment,
				queueFamilyIndices.isSame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,familyIndicesArr,surfaceCapabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,presentMode,1U,oldSwapchain
			);
			
			swapchain = device.createSwapchainKHR(info);
			if (!swapchain) throw std::runtime_error("create SwapChain failed!");

			swapchainImages = device.getSwapchainImagesKHR(swapchain);
			surfaceFormat = format.format;
			surfaceExtent = extent;
		}
		void createSwapchainImageViews()
		{
			swapChainImageViews.clear();
			swapChainImageViews.resize(swapchainImages.size());

			for (int i = 0; i < swapChainImageViews.size(); ++i)
			{
				vk::ImageViewCreateInfo info(vk::ImageViewCreateFlags(),swapchainImages[i],vk::ImageViewType::e2D,surfaceFormat,
				vk::ComponentMapping(),{vk::ImageAspectFlagBits::eColor,0,1,0,1});
				swapChainImageViews[i] = device.createImageView(info);
			}
		}

		virtual vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
		{
			if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
				return vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear);
			for (const auto& f : formats)
			{
				if (f.format == vk::Format::eR8G8B8A8Unorm && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
					return f;
			}
			return formats[0];
		}
		virtual vk::PresentModeKHR chooseSwapPresent(const std::vector<vk::PresentModeKHR>&presentMods)
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
		virtual vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
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

		virtual uint32_t onSetSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
		{
			uint32_t imageCount = capabilities.minImageCount + 1;
			if (capabilities.maxImageCount > 0 && imageCount > capabilities.minImageCount)
				imageCount = capabilities.maxImageCount;
			return imageCount;
		}

		template<typename FT = vk::Format,typename TI = vk::ImageTiling,typename FE = vk::FormatFeatureFlagBits,TI Ti,FE Fe,FT FFt,FT...Ft>
		FT findSupportedFormat() {
			
			vk::FormatProperties prop = physicalDevice.getFormatProperties(FFt);

			if (Ti == vk::ImageTiling::eLinear && (prop.linearTilingFeatures & Fe) == Fe) {
				return FFt;
			}
			else if (Ti == vk::ImageTiling::eOptimal && (prop.optimalTilingFeatures & Fe) == Fe) {
				return FFt;
			}

			if constexpr (sizeof...(Ft) > 0)
			{
				return findSupportedFormat<FT,TI,FE,Ti,Fe,Ft...>();
			}
			else {
				throw std::runtime_error("failed to find supported format!");
			}
		}

		virtual vk::Format onChooseDepthStencilFormat()
		{
			return findSupportedFormat<vk::Format,vk::ImageTiling,vk::FormatFeatureFlagBits,vk::ImageTiling::eOptimal,vk::FormatFeatureFlagBits::eDepthStencilAttachment,
				vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD16Unorm>();
		}

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlagBits properties) {
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

		void createDepthStencilAttachment()
		{
			depthFormat = onChooseDepthStencilFormat();
			vk::ImageCreateInfo imgInfo({},vk::ImageType::e2D,depthFormat,vk::Extent3D(surfaceExtent.width,surfaceExtent.height,1),1,1,vk::SampleCountFlagBits::e1,vk::ImageTiling::eOptimal,
				vk::ImageUsageFlagBits::eDepthStencilAttachment,vk::SharingMode::eExclusive,{});
			depthAttachment.image = device.createImage(imgInfo);
			auto imgAspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			if (eq_enum<vk::Format, vk::Format::eD16Unorm, vk::Format::eD32Sfloat>(depthFormat))
				imgAspect = vk::ImageAspectFlagBits::eDepth;
			auto req = device.getImageMemoryRequirements(depthAttachment.image);
			
			vk::MemoryAllocateInfo allocInfo(req.size,findMemoryType(req.memoryTypeBits,vk::MemoryPropertyFlagBits::eDeviceLocal));
			depthAttachment.mem = device.allocateMemory(allocInfo);

			device.bindImageMemory(depthAttachment.image,depthAttachment.mem,0);

			vk::ImageViewCreateInfo viewInfo({},depthAttachment.image,vk::ImageViewType::e2D,depthFormat,{},vk::ImageSubresourceRange(imgAspect,0,1,0,1));
			depthAttachment.view = device.createImageView(viewInfo);
		}

		virtual void createRenderPass()
		{
			auto depthImageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			if(eq_enum<vk::Format,vk::Format::eD16Unorm,vk::Format::eD32Sfloat>(depthFormat))
				depthImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			std::vector<vk::AttachmentDescription> attachment = {
				vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),surfaceFormat,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,vk::ImageLayout::ePresentSrcKHR),
				vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),depthFormat,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eDontCare,vk::ImageLayout::eUndefined,depthImageLayout)
			};
			 
			vk::AttachmentReference attachmentRef(0,vk::ImageLayout::eColorAttachmentOptimal);
			vk::AttachmentReference depthAttachmentRef(1,depthImageLayout);


			std::vector<vk::SubpassDescription> subpassDesc = {
				vk::SubpassDescription({},vk::PipelineBindPoint::eGraphics,{}, &attachmentRef,{}, &depthAttachmentRef)
			};

			vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(),attachment,subpassDesc);
			renderPass = device.createRenderPass(info);
		}

		void createFramebuffers()
		{
			std::vector<vk::ImageView> attachments = { swapChainImageViews[0],depthAttachment.view };
			vk::FramebufferCreateInfo info({},renderPass,attachments,surfaceExtent.width,surfaceExtent.height,1);
			framebuffers.clear();
			framebuffers.reserve(swapChainImageViews.size());
			for (int i = 0; i < swapChainImageViews.size(); ++i)
			{
				attachments[0] = swapChainImageViews[i];
				framebuffers.push_back(device.createFramebuffer(info));
			}
		}

		void createCommandPool()
		{
			vk::CommandPoolCreateInfo info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer ,queueFamilyIndices.graphicsFamily);
			commandPool = device.createCommandPool(info);
		}

		void createCommandBuffers()
		{
			vk::CommandBufferAllocateInfo info(commandPool,vk::CommandBufferLevel::ePrimary,framebuffers.size());
			 commandbuffers = device.allocateCommandBuffers(info);
		}
		void createSemaphores()
		{
			vk::SemaphoreCreateInfo info;
			acquired_image_ready = device.createSemaphore(info);
			render_complete = device.createSemaphore(info);
		}
		

		virtual void drawFrame()
		{
			auto[res,imageIndex] = device.acquireNextImageKHR(swapchain,std::numeric_limits<uint64_t>::max(),acquired_image_ready);

			if (res == vk::Result::eErrorOutOfDateKHR)
			{
				if(!minWindowSize)
					recreateSwapChain();
			}
			else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
			{
				throw std::runtime_error("failed to present swap chain image!");
			}
			auto cmdBuf = commandbuffers[imageIndex];
			onDraw(cmdBuf,framebuffers[imageIndex]);
			vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			vk::SubmitInfo info(1,&acquired_image_ready,&waitStage,1,&cmdBuf,1,&render_complete);
			vk::Fence drawFence = device.createFence(vk::FenceCreateInfo());
			graphicsQueue.submit(info,drawFence);
			vk::Result presentRes;
			vk::PresentInfoKHR presentInfo(1,&render_complete,1,&swapchain,&imageIndex,&presentRes);

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
			while (vk::Result::eTimeout == device.waitForFences(drawFence, VK_TRUE, std::numeric_limits<std::uint64_t>::max()));
			device.destroyFence(drawFence);
		}

		virtual void onDraw(vk::CommandBuffer& cmd,vk::Framebuffer frameBuf)
		{
			vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			cmd.begin(beginInfo);
			std::array<vk::ClearValue,2> clearVals = { clearColorValue,vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0)) };
			vk::RenderPassBeginInfo renderPassBeginInfo(renderPass,frameBuf,vk::Rect2D({0,0},surfaceExtent),clearVals);
			cmd.beginRenderPass(renderPassBeginInfo,vk::SubpassContents::eInline);

			onRealDraw(cmd);

			cmd.endRenderPass();
			cmd.end();
		}

		virtual void onRealDraw(vk::CommandBuffer& cmd)=0;

		void recreateSwapChain()
		{
			device.waitIdle();

			createSwapChain(true);
			createSwapchainImageViews();
			createFramebuffers();
			createDepthStencilAttachment();
			createRenderPass();
			onReCreateSwapChain();
		}
		
		virtual void onInit() = 0;
		virtual void onCreate(){};
		virtual void onCreateWindow(){};
		virtual void onUpdate(float delta) {};
		virtual void onWindowResize(uint32_t w, uint32_t h)
		{
			width = w;height =  h;
			minWindowSize = (w == 0 || h == 0);
			if(minWindowSize)
				return;
			recreateSwapChain();
		}
		virtual void onReCreateSwapChain() = 0;
		virtual void onFillValidationLayers(std::vector<const char*>& vec)
		{
			vec.push_back("VK_LAYER_KHRONOS_validation");
		}
		virtual void onFillDeviceNeedExtensions(std::vector<const char*>& vec)
		{
			vec.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
		virtual void onSetPhysicalDeviceFeatures(const vk::PhysicalDeviceFeatures features){}
		virtual VkBool32 onDebugReportError(
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage) {
			return false;
		}
		virtual vk::PhysicalDevice onPickPhysicalDevice(const std::vector<std::tuple<vk::PhysicalDevice,QueueFamilyIndices>>& devices)
		{
			return std::get<0>(devices[0]);
		}

		protected:

		GLFWwindow* window;
		VkSurfaceKHR surface;
		bool enableValidationLayers = false;
		bool minWindowSize = false;
		float lastFrameDelta = 0.0f;
		const char* sample_name = "";
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue,presentQueue;
		vk::SwapchainKHR swapchain;
		vk::Format surfaceFormat;
		vk::Format depthFormat = vk::Format::eD24UnormS8Uint;
		vk::Extent2D surfaceExtent;
		vk::RenderPass renderPass;
		vk::Semaphore render_complete, acquired_image_ready;
		struct 
		{
			vk::Image image;
			vk::ImageView view;
			vk::DeviceMemory mem;
		} depthAttachment;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapChainImageViews;
		std::vector<vk::Framebuffer> framebuffers;
		std::vector<vk::CommandBuffer> commandbuffers;
		vk::CommandPool commandPool;
		uint32_t width,height;
		std::optional<std::vector<const char*>> ValidationLayers;
		std::vector<const char*> DeviceNeedExtensions;
		VkDebugReportCallbackEXT debugReport;
		std::array<float,4> clearColorArr{ 1.0f,0.0f,0.0f,1.0f };
		vk::ClearValue clearColorValue;
		
		QueueFamilyIndices queueFamilyIndices;

		VarCache<vk::PhysicalDeviceMemoryProperties,vk::PhysicalDevice> memPropCache;

		static void WindowReSize(GLFWwindow* window, int w, int h);
		static VkBool32 DebugReportCallbackEXT(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData);
	};
}