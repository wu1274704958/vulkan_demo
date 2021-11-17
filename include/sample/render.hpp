#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <common.hpp>
#include <optional>
#include <json.hpp>
#include <event/event.hpp>
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
	struct Component;
	struct Object;

	class SampleRender : public evt::EventDispatcher{
		friend Component;
		friend Object;
	public:
		SampleRender() 
			: memPropCache(std::function([](std::tuple<vk::PhysicalDevice&>& args){
				return std::get<0>(args).getMemoryProperties();
			}),physicalDevice),
			clearColorValue(clearColorArr) {
			}
		SampleRender(bool enableValidationLayers,const char* sample_name) 
			: memPropCache(std::function([](std::tuple<vk::PhysicalDevice&>& args) {
				return std::get<0>(args).getMemoryProperties();
			}), physicalDevice),
			clearColorValue(clearColorArr)
		{

			this->enableValidationLayers = enableValidationLayers;
			this->sample_name = sample_name;
		}
		SampleRender(const SampleRender&) = delete;
		SampleRender(SampleRender&&) = delete;
		virtual ~SampleRender();
		void init(int w,int h);
		virtual void mainLoop();
		void cleanUp() ;
		virtual bool dispatchEvent(const evt::Event&) override ;
	protected:
		void initWindow(uint32_t w, uint32_t h);
		bool checkValidationLayerSupport() ;
		std::vector<const char*> getRequiredExtensions();
		void createInstance();
		void setUpDebugCallback();
		void createSurface();
		QueueFamilyIndices pickPhysicalDevice();
		std::tuple<bool,QueueFamilyIndices> isDeviceSuitable(const vk::PhysicalDevice& d);
		bool checkDeviceExtensionSupport(const vk::PhysicalDevice& d);
		QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& d);
		SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& d);
		void createLogicalDevice(const QueueFamilyIndices& indices);
		void createSwapChain(bool recreate = false);
		void createSwapchainImageViews();
		virtual vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
		virtual vk::PresentModeKHR chooseSwapPresent(const std::vector<vk::PresentModeKHR>&presentMods);
		virtual vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
		virtual uint32_t onSetSwapChainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);
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
		virtual vk::Format onChooseDepthStencilFormat();
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlagBits properties);
		void createDepthStencilAttachment();
		virtual void createRenderPass();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSemaphores();
		void createDrawFences();
		virtual void drawFrame();
		virtual void onDraw(vk::CommandBuffer& cmd,vk::Framebuffer frameBuf);
		virtual void onRealDraw(vk::CommandBuffer& cmd)=0;
		void recreateSwapChain();
		virtual void onCleanUpPipeline() = 0;
		void cleanUpSwapChain();
		virtual void onCleanUp() = 0;
		virtual void onInit() = 0;
		virtual void onCreate();
		virtual void onCreateWindow();
		virtual void onUpdate(float delta);
		virtual void onWindowResize(uint32_t w, uint32_t h);
		virtual void onReCreateSwapChain() = 0;
		virtual void onFillValidationLayers(std::vector<const char*>& vec);
		virtual void onFillDeviceNeedExtensions(std::vector<const char*>& vec);
		virtual void onSetPhysicalDeviceFeatures(const vk::PhysicalDeviceFeatures features);
		virtual VkBool32 onDebugReportError(
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage);
		virtual std::tuple<vk::PhysicalDevice, QueueFamilyIndices> onPickPhysicalDevice(const std::vector<std::tuple<vk::PhysicalDevice,QueueFamilyIndices>>& devices);
	public:
		EngineState engineState = EngineState::Uninitialized;
	protected:
		GLFWwindow* window;
		VkSurfaceKHR surface;
		bool enableValidationLayers = false;
		bool minWindowSize = false;
		bool isInit = false;
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
		std::vector<vk::Fence> drawFences;
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
		std::array<float,4> clearColorArr{ 0.1019607f,0.10980392f,0.12941176f,1.0f };
		vk::ClearValue clearColorValue;
		evt::GlfwEventConstructor eventConstructor;
		QueueFamilyIndices queueFamilyIndices;
		wws::VarCache<vk::PhysicalDeviceMemoryProperties,vk::PhysicalDevice> memPropCache;
		static VkBool32 DebugReportCallbackEXT(
			VkDebugReportFlagsEXT                       flags,
			VkDebugReportObjectTypeEXT                  objectType,
			uint64_t                                    object,
			size_t                                      location,
			int32_t                                     messageCode,
			const char* pLayerPrefix,
			const char* pMessage,
			void* pUserData);
		static SampleRender* self_instance;
	};
}