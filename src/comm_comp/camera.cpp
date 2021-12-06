#include <comm_comp/scene.hpp>
#include <comm_comp/camera.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>
namespace vkd
{
	void Camera::attach_scene(const std::weak_ptr<Scene>& scene)
	{
		auto sc = scene.lock();
		sc->add_camera(std::dynamic_pointer_cast<Camera>(shared_from_this()));
		attached = true;
	}

	void Camera::detach_scene()
	{
		if(!attached)  return;
		auto trans = object.lock()->get_comp_raw<Transform>();
		if(trans)
		{
			if (auto scene = trans->get_scene().lock();scene)
				scene->rm_camera(std::dynamic_pointer_cast<Camera>(shared_from_this()));
		}
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
