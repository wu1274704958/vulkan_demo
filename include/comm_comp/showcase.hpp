#pragma once

#include <core/component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkd {
	struct Camera : public  Component
	{
		virtual const glm::mat4& get_matrix_p() const = 0;
		virtual const glm::mat4& get_matrix_v() const = 0;
	protected:
	};

	struct Showcase : public Camera
	{
		const glm::mat4& get_matrix_p() const override;
		const glm::mat4& get_matrix_v() const override;
		bool dispatchEvent(const evt::Event&) override;
	protected:
		bool on_init() override{}
		void on_clean_up() override {}
		void awake() override;
		void recreate_swapchain() override;
		void update(float delta) override;
		void attach_scene() override;
		void detach_scene() override;
	protected:
		glm::mat4 mat_p;
		glm::vec2 mouseLastPos, mouseMoveOffset;
		float fovy = 45.0f,zNera = 0.1f,zFar = 100.0f;
	};
}