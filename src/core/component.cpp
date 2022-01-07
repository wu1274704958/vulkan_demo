#include <core/component.hpp>
#include <sample/render.hpp>
namespace vkd{

	Component::Component()
	{
		
	}

	Component::Component(const Component& oth)
	{
		this->enable = oth.enable;
		this->ever_tick = oth.ever_tick;
		this->not_draw = oth.not_draw;
	}


	bool Component::is_enable() { return enable; }
	void Component::set_enable(bool v) { enable = v; v ? on_enable() : on_disable(); }
	bool Component::is_ever_tick() { return ever_tick; }
	void Component::set_ever_tick(bool v) { ever_tick = v; }
	int64_t Component::idx() const { return 10000; }
	bool Component::initialized() const {return is_init;}
	Component::~Component() = default;
	std::shared_ptr<Object> Component::get_object() const { return object.lock(); }

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

	vk::RenderPassBeginInfo Component::render_pass_begin_info()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->renderPassBeginInfo;
		return {};
	}

	vk::Framebuffer Component::curr_frame_buffer()
	{
		if (SampleRender::self_instance) return SampleRender::self_instance->currFrameBuffer;
		return {};
	}





}