#include <comm_comp/transform.hpp>
#include <core/object.hpp>

namespace vkd {
	Transform::Transform() : Component(),mat(1.0f)
	{
		ever_tick = true;
	}

	void Transform::set_enable(bool v)
	{
		if(v) Component::set_enable(v);
	}

	bool Transform::on_init()
	{
		bool res = true;
		for(auto& ch : childlren)
		{
			auto obj = ch->object.lock();
			if(obj && obj->is_active())
			{
				if(!obj->init())  res = false;
			}
		}
		return res;
	}



}
