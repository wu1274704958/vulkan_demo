#include <comm_comp/transform.hpp>
#include <core/object.hpp>

namespace vkd {
	Transform::Transform() : Component(),position(0.0f),rotation(0.0f),
		scale(1.0f),mat(1.0f)
	{
		ever_tick = true;
	}

	void Transform::set_enable(bool v)
	{
		Component::set_enable(v);
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj )
			{
				obj->set_active(v);
			}
		}
	}

	bool Transform::on_init()
	{
		bool res = true;
		for(const auto& ch : childlren)
		{
			if(const auto obj = ch->object.lock();obj && obj->is_active())
			{
				if(!obj->init())  res = false;
			}
		}
		return res;
	}

	void Transform::recreate_swapchain()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->recreate_swapchain();
			}
		}
	}

	void Transform::draw(vk::CommandBuffer& cmd)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->draw(cmd);
			}
		}
	}

	void Transform::update(float delta)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->update(delta);
			}
		}
	}

	void Transform::late_update(float delta)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->late_update(delta);
			}
		}
	}

	void Transform::on_clean_up()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->clean_up();
			}
		}
	}

	void Transform::clean_up_pipeline()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->clean_up_pipeline();
			}
		}
	}

	bool Transform::dispatchEvent(const evt::Event& e)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				if(obj->dispatchEvent(e)) return true;
			}
		}
		return false;
	}

	const glm::vec3& Transform::get_position()const { return position; }
	const glm::vec3& Transform::get_rotation()const { return rotation; }
	const glm::vec3& Transform::get_scale()   const { return scale; }
	void Transform::set_position(glm::vec3 pos) { position = pos;dirty = true; }
	void Transform::set_rotation(glm::vec3 rot) { rotation = rot; dirty = true; }
	void Transform::set_scale(glm::vec3 scale) { this->scale = scale; dirty = true; }
}
