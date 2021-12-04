#pragma once

#include <string>
#include <core/object.hpp>
#include <core/component.hpp>
#include <comm_comp/pipeline.hpp>
#include <res_loader/data_mgr.hpp>

namespace vkd
{
	struct Texture : public vkd::Component
	{
		Texture(std::string path,uint16_t set = 0,uint32_t binding = 1) : path(path),set(set),binding(binding){}
		void update_descriptor() const;
		bool on_init() override;
		void recreate_swapchain() override;
		void on_clean_up() override;
		void awake() override;
	protected:
		std::shared_ptr<gld::vkd::VkdImage> img;
		std::string path;
		uint16_t set;
		uint32_t binding;
	};
}
