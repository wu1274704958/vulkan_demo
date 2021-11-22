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
	}

	void Camera::detach_scene()
	{
		auto trans = object.lock()->get_comp_raw<Transform>();
		if(trans)
		{
			if (auto scene = trans->get_scene().lock();scene)
				scene->rm_camera(std::dynamic_pointer_cast<Camera>(shared_from_this()));
		}
	}

	void Camera::on_destroy(bool with_obj)
	{
		if(!with_obj)
			detach_scene();
		Component::on_destroy(with_obj);
	}

}
