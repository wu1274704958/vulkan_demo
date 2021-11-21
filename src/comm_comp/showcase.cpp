#include <comm_comp/scene.hpp>
#include <comm_comp/showcase.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkd
{
	void Showcase::awake()
	{
		auto surface = surface_extent();
		mat_p = glm::perspective(glm::radians(fovy), static_cast<float>(surface.width) / static_cast<float>(surface.height), 
				zNera, zFar);
	}

	bool Showcase::dispatchEvent(const evt::Event& e)
	{
		switch (e.type)
		{
		case vkd::evt::EventType::MouseDown:
		{
			auto& ev = e.GetEvent<vkd::evt::MouseButtonEvent>();
			mouseLastPos.x = (float)ev.x;
			mouseLastPos.y = (float)ev.y;
		}
		break;
		case vkd::evt::EventType::MouseMove:
			if (event_constructor().isMouseBtnPress(vkd::evt::MouseBtnLeft))
			{
				auto& ev = e.GetEvent<vkd::evt::MouseButtonEvent>();
				auto surface = surface_extent();
				mouseMoveOffset.x = ((float)ev.x - mouseLastPos.x) / surface.width;
				mouseMoveOffset.y = ((float)ev.y - mouseLastPos.y) / surface.height;
				mouseLastPos.x = (float)ev.x;
				mouseLastPos.y = (float)ev.y;
				return true;
			}
			break;
		case vkd::evt::EventType::MouseUp:
			mouseMoveOffset.y = mouseMoveOffset.x = 0.0f;
			break;
		default:
			break;
		}
		return false;
	}

	void Showcase::recreate_swapchain()
	{
		awake();
	}

	const glm::mat4& Showcase::get_matrix_p() const
	{
		return mat_p;
	}

	const glm::mat4& Showcase::get_matrix_v() const
	{
		return object.lock()->get_comp_raw<Transform>()->get_matrix();
	}

	void Showcase::update(float delta)
	{
		glm::vec2 off = mouseMoveOffset * delta * 42000.0f;
		if(auto obj = object.lock();obj)
		{
			if(auto trans = obj->get_comp_raw<Transform>();trans)
			{
				auto z = trans->get_rotation().z;
				trans->set_rotation(glm::vec3(off.x,off.y,z));
			}
		}
	}

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
