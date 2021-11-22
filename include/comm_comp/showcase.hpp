#pragma once

#include <core/component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <comm_comp/camera.hpp>

namespace vkd {

	struct Showcase : public Camera
	{
		const glm::mat4& get_matrix_p() const override;
		const glm::mat4& get_matrix_v() const override;
		bool dispatchEvent(const evt::Event&) override;
		void awake() override;
		bool on_init() override{return true;}
		void on_clean_up() override {}
		void recreate_swapchain() override;
		void update(float delta) override;
		void draw(vk::CommandBuffer& cmd) override{}
	protected:
		glm::mat4 mat_p;
		glm::vec2 mouseLastPos, mouseMoveOffset;
		float fovy = 45.0f,zNera = 0.1f,zFar = 100.0f;
	};
}