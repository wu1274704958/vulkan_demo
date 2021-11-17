#pragma once

#include <core/component.hpp>

namespace vkd {

	struct Showcase : public Component
	{
		virtual void awake() override
		{
		}
		virtual bool on_init() override
		{
			return false;
		}
		virtual void on_enable() override
		{
		}
		virtual void on_disable() override
		{
		}
		virtual void recreate_swapchain() override
		{
		}
		virtual void draw(vk::CommandBuffer& cmd) override
		{
		}
		virtual void update(float delta) override
		{
		}
		virtual void late_update(float delta) override
		{
		}
		virtual void on_clean_up() override
		{
		}
		virtual void clean_up_pipeline() override
		{
		}
		virtual int64_t idx() override {
			return 1l;
		}
	};
}