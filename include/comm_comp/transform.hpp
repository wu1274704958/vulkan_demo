#pragma once

#include <core/component.hpp>
#include <numeric>
#include <res_loader/data_mgr.hpp>
#include <glm/glm.hpp>


namespace vkd {

	struct Transform : public Component{
		Transform();
		bool on_init() override;
		void on_clean_up() override;
		void set_enable(bool v) override;
		void recreate_swapchain() override;
		void draw(vk::CommandBuffer& cmd) override;
		void update(float delta) override;
		void late_update(float delta) override;
		void on_clean_up() override;
		void clean_up_pipeline() override;
		int64_t idx() override { return 0; }
	protected:
		glm::vec4 mat;
		std::vector<std::shared_ptr<Transform>> childlren;
	};

}