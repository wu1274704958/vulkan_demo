#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <common.hpp>
#include <optional>
#include <json.hpp>
namespace vkd {

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
		SampleRender(){}
		SampleRender(bool enableValidationLayers,const char* sample_name) {
			this->enableValidationLayers = enableValidationLayers;
			this->sample_name = sample_name;
		}
		SampleRender(const SampleRender&) = delete;
		SampleRender(SampleRender&&) = delete;
		~SampleRender()
		{
			
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
			onInit();
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
			if (instance)
			{
				throw std::runtime_error("Create instance failed!");
			}
		}
		void setUpDebugCallback()
		{
			if (!enableValidationLayers)
				return;
			vk::DebugReportCallbackCreateInfoEXT info(vk::DebugReportFlagsEXT(), DebugReportCallbackEXT,(void*)this);
			if (instance.createDebugReportCallbackEXT(info))
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

		void createSwapChain()
		{
			auto format = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
			auto presentMode = chooseSwapPresent(physicalDevice.getSurfacePresentModesKHR(surface));
			auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
			auto extend = chooseSwapExtent(surfaceCapabilities);
			const uint32_t temp [] = {queueFamilyIndices.graphicsFamily,queueFamilyIndices.presentFamily};
			vk::ArrayProxyNoTemporaries<const uint32_t> familyIndicesArr = queueFamilyIndices.isSame() ? vk::ArrayProxyNoTemporaries(temp[0]) :
				vk::ArrayProxyNoTemporaries(2,temp);
			vk::SwapchainCreateInfoKHR info(vk::SwapchainCreateFlagsKHR(),surface,onSetSwapChainMinImageCount(surfaceCapabilities),format.format,
				format.colorSpace,extend,1,vk::ImageUsageFlags(),
				queueFamilyIndices.isSame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,familyIndicesArr,surfaceCapabilities.currentTransform,
				vk::CompositeAlphaFlagBitsKHR::eOpaque,presentMode,1U
			);
			
			swapchain = device.createSwapchainKHR(info);
			if (!swapchain) throw std::runtime_error("create SwapChain failed!");

			swapchainImages = device.getSwapchainImagesKHR(swapchain);
			surfaceFormat = format.format;
			surfaceExtent = extend;
		}
		void createSwapchainImageViews()
		{
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

		void createRenderPass()
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

			//vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(),attachment,)
		}

		virtual void onInit() = 0;
		virtual void onCreate() = 0;
		virtual void onCreateWindow() = 0;
		virtual void onWindowResize(uint32_t w, uint32_t h)
		{
			width = w;height =  h;
		}
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
			const char* pMessage) = 0;
		virtual vk::PhysicalDevice onPickPhysicalDevice(const std::vector<std::tuple<vk::PhysicalDevice,QueueFamilyIndices>>& devices)
		{
			return std::get<0>(devices[0]);
		}

		protected:

		GLFWwindow* window;
		VkSurfaceKHR surface;
		bool enableValidationLayers = false;
		const char* sample_name = "";
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue,presentQueue;
		vk::SwapchainKHR swapchain;
		vk::Format surfaceFormat;
		vk::Format depthFormat = vk::Format::eD24UnormS8Uint;
		vk::Extent2D surfaceExtent;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapChainImageViews;
		uint32_t width,height;
		std::optional<std::vector<const char*>> ValidationLayers;
		std::vector<const char*> DeviceNeedExtensions;
		QueueFamilyIndices queueFamilyIndices;

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