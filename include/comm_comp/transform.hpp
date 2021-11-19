#pragma once

#include <core/component.hpp>
#include <numeric>
#include <res_loader/data_mgr.hpp>
#include <glm/glm.hpp>


namespace vkd {

	struct Transform : public Component{
		Transform();
		void awake() override{}
		bool on_init() override;
		void set_enable(bool v) override;
		void recreate_swapchain() override;
		void draw(vk::CommandBuffer& cmd) override;
		void update(float delta) override;
		void late_update(float delta) override;
		void on_clean_up() override;
		void clean_up_pipeline() override;
		bool dispatchEvent(const evt::Event&) override;
		int64_t idx() override { return std::numeric_limits<int64_t>::max(); }
		
		const glm::vec3& get_position()const;
		const glm::vec3& get_rotation()const;
		const glm::vec3& get_scale()   const;

		void set_position(glm::vec3 pos);
		void set_rotation(glm::vec3 rot);
		void set_scale(glm::vec3 scale);

	protected:
		glm::vec3 position,rotation,scale;
		glm::vec4 mat;
		std::vector<std::shared_ptr<Transform>> childlren;
		bool dirty:1 = true;
	};

}