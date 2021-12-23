#include <comm_comp/scene.hpp>
#include <comm_comp/camera.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>
namespace vkd
{
	void Camera::attach_scene(const std::weak_ptr<Scene>& scene)
	{
		auto sc = scene.lock();
		sc->add_bind_comp<Camera>(std::dynamic_pointer_cast<Camera>(shared_from_this()));
		attached = true;
	}

	void Camera::detach_scene()
	{
		attached = false;
	}

	void Camera::on_destroy()
	{
		detach_scene();
	}

	bool Camera::is_dirty() const
	{
		return dirty;
	}

	void Camera::awake()
	{
		not_draw = true;
	}



}
