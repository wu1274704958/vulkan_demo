#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include <event/event.hpp>

namespace vkd {

	struct Object;

	struct Component : public evt::EventDispatcher
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

	protected:
		virtual void awake() = 0;
		bool init() {
			auto v = on_init();
			is_init = true;
			return v;
		}
		virtual bool on_init() = 0;
		virtual void on_enable() = 0;
		virtual void on_disable() = 0;
		virtual void recreate_swapchain() = 0;
		virtual void attach_object(std::weak_ptr<Object> n) { object = n; }
		virtual void reset_object() { object.reset(); }
		virtual void draw(vk::CommandBuffer& cmd) = 0;
		virtual void update(float delta) = 0;
		virtual void late_update(float delta) = 0;
		void clean_up() {
			clean_up();
			is_init = false;
		}
		virtual void on_clean_up()=0;
		virtual void clean_up_pipeline() = 0;
		virtual void on_destroy(){
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
		bool is_init : 1 = false;
	};

}