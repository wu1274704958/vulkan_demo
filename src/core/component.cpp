#include <core/component.hpp>
#include <sample/render.hpp>
namespace vkd{
	vk::Instance Component::instance()
	{
		if(SampleRender::self_instance) return SampleRender::self_instance->instance;
	}
	vk::Device Component::device()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->device;
	}
	vk::PhysicalDevice Component::physical_dev(){
		if (SampleRender::self_instance) return SampleRender::self_instance->physicalDevice;
	}
	vk::CommandPool Component::command_pool() {
		if (SampleRender::self_instance) return SampleRender::self_instance->commandPool;
	}
	vk::Queue Component::graphics_queue(uint32_t i) {
		if (SampleRender::self_instance) return SampleRender::self_instance->graphicsQueue;
	}
	vk::Queue Component::present_queue(uint32_t i) {
		if (SampleRender::self_instance) return SampleRender::self_instance->presentQueue;
	}
	vk::Format Component::surface_format() {
		if (SampleRender::self_instance) return SampleRender::self_instance->surfaceFormat;
	}
	vk::Format Component::depthstencil_format() {
		if (SampleRender::self_instance) return SampleRender::self_instance->depthFormat;
	}
	const vk::Extent2D& Component::surface_extent() {
		if (SampleRender::self_instance) return SampleRender::self_instance->surfaceExtent;
	}
	vk::RenderPass Component::renderpass() {
		if (SampleRender::self_instance) return SampleRender::self_instance->renderPass;
	}

}