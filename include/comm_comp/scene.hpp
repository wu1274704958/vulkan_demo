#pragma once
#include <core/component.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>

namespace vkd
{
	struct Camera;

	struct Scene : public Transform
	{
		std::weak_ptr<Scene> get_scene() override;

		std::shared_ptr<const Camera> get_camera() const;
		void add_camera(std::weak_ptr<Camera> cam);
		bool has_camera(std::weak_ptr<Camera> cam) const;
		bool rm_camera(std::shared_ptr<Camera> cam);
	protected:
		std::vector<std::weak_ptr<Camera>> cameras;
	};	
}
