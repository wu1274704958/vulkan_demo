#pragma once

#include <core/component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkd {
	struct Camera : public  Component
	{
		virtual const glm::mat4& get_matrix_p() const = 0;
		virtual const glm::mat4& get_matrix_v() const = 0;
		void attach_scene(const std::weak_ptr<Scene>& scene) override;
		void detach_scene() override;
		void on_destroy(bool with_obj) override;
	protected:
	};

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