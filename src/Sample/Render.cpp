#include <sample/render.hpp>

void vkd::SampleRender::WindowReSize(GLFWwindow* window, int w, int h)
{
	auto ptr = (SampleRender*)glfwGetWindowUserPointer(window);
	if (w == 0 || h == 0 || ptr == nullptr)
		return;
	ptr->onWindowResize((uint32_t)w, (uint32_t)h);
}

VkBool32 vkd::SampleRender::DebugReportCallbackEXT(
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

VkResult vkd::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
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

void vkd::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT pCallback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCallback, pAllocator);
	}
}