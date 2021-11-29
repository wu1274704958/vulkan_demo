#pragma once

#include <core/component.hpp>

namespace vkd {
	struct Camera : public  Component
	{
		virtual const glm::mat4& get_matrix_p() const = 0;
		virtual const glm::mat4& get_matrix_v() const = 0;
		virtual bool is_dirty() const;
		void attach_scene(const std::weak_ptr<Scene>& scene) override;
		void detach_scene() override;
		void on_destroy(bool with_obj) override;
		void awake() override;
	protected:
		bool dirty:1 = true;
	};
}