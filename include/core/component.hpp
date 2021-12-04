#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include <event/event.hpp>

namespace vkd {
	struct DepthAttachment;
	struct Object;
	struct Scene;
	struct Component : public std::enable_shared_from_this<Component>, public evt::EventDispatcher
	{
		friend Object;
		virtual bool is_enable(){return enable;}
		virtual void set_enable(bool v) { enable = v; v ? on_enable() : on_disable();}
		virtual bool is_ever_tick() { return ever_tick; }
		virtual void set_ever_tick(bool v) { ever_tick = v; }
		virtual int64_t idx() { return 0; }

		virtual ~Component() {}
		std::shared_ptr<Object> get_object() { return object.lock(); }

		static vk::Instance instance();
		static vk::Device device();
		static vk::PhysicalDevice physical_dev();
		static vk::CommandPool command_pool();
		static vk::Queue graphics_queue(uint32_t i = 0);
		static vk::Queue present_queue(uint32_t i = 0);
		static vk::Format surface_format();
		static vk::Format depthstencil_format();
		static const vk::Extent2D& surface_extent();
		static vk::RenderPass renderpass();
		static evt::GlfwEventConstructor event_constructor();
		static const DepthAttachment& depth_attachment();

	protected:
		virtual void awake(){}
		bool init() {
			auto v = on_init();
			is_init = true;
			return v;
		}
		virtual bool on_init() = 0;
		virtual void on_enable(){}
		virtual void on_disable() {}
		virtual void recreate_swapchain(){}
		virtual void attach_object(std::weak_ptr<Object> n) { object = n; }
		virtual void detach_object() { object.reset(); }
		virtual void draw(vk::CommandBuffer& cmd) {}
		virtual void update(float delta){}
		virtual void late_update(float delta) {}
		virtual void attach_scene(const std::weak_ptr<Scene>& scene) {}
		virtual void detach_scene() {}
		void clean_up() {
			on_clean_up();
			is_init = false;
		}
		virtual void on_clean_up()=0;
		virtual void clean_up_pipeline(){}
		virtual void on_destroy(bool with_obj){
			if(is_init)
			{ 
				clean_up_pipeline();
				clean_up();
				is_init = false;
			}
		}

		std::weak_ptr<Object> object;
		bool enable : 1 = false;
		bool ever_tick : 1 = false;
		bool not_draw : 1 = false;
		bool is_init : 1 = false;
	};

}