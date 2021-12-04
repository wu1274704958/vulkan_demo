#include <core/component.hpp>
#include <sample/render.hpp>
namespace vkd{
	vk::Instance Component::instance()
	{
		if(SampleRender::self_instance) return SampleRender::self_instance->instance;
		return {};
	}
	vk::Device Component::device()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->device;
		return {};
	}
	vk::PhysicalDevice Component::physical_dev(){
		if (SampleRender::self_instance) return SampleRender::self_instance->physicalDevice;
		return {};
	}
	vk::CommandPool Component::command_pool() {
		if (SampleRender::self_instance) return SampleRender::self_instance->commandPool;
		return {};
	}
	vk::Queue Component::graphics_queue(uint32_t i) {
		if (SampleRender::self_instance) return SampleRender::self_instance->graphicsQueue;
		return {};
	}
	vk::Queue Component::present_queue(uint32_t i) {
		if (SampleRender::self_instance) return SampleRender::self_instance->presentQueue;
		return {};
	}
	vk::Format Component::surface_format() {
		if (SampleRender::self_instance) return SampleRender::self_instance->surfaceFormat;
		return {};
	}
	vk::Format Component::depthstencil_format() {
		if (SampleRender::self_instance) return SampleRender::self_instance->depthFormat;
		return {};
	}
	const vk::Extent2D& Component::surface_extent() {
		if (SampleRender::self_instance) return SampleRender::self_instance->surfaceExtent;
		return {};
	}
	vk::RenderPass Component::renderpass() {
		if (SampleRender::self_instance) return SampleRender::self_instance->renderPass;
		return {};
	}
	evt::GlfwEventConstructor Component::event_constructor()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->eventConstructor;
		return {};
	}

	const DepthAttachment& Component::depth_attachment()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->depthAttachment;
		return {};
	}



}