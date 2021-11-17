#include "core/object.hpp"

namespace vkd {

	uint32_t Object::component_count() { return components.size(); }
	void Object::adjust_locat(size_t i, int offset)
	{
		for (; i < components.size(); ++i)
		{
			size_t ty_id = typeid(*components[i]).hash_code();
			locator[ty_id] += offset;
		}
	}

	bool Object::good_comp_idx(int idx)
	{
		return components.size() > idx;
	}
	std::shared_ptr<Object> Object::ptr()
	{
		return std::enable_shared_from_this<Object>::shared_from_this();
	}
	std::weak_ptr<Object> Object::weak_ptr()
	{
		return std::enable_shared_from_this<Object>::weak_from_this();
	}

	void Object::set_active(bool v)
	{
		if (v != active)
		{
			active = v;
			for (auto& comp : components)
			{
				if(comp->is_enable())
					v ? comp->on_enable() : comp->on_disable();
			}
		}
	}
	bool Object::is_active()
	{
		return active;
	}

	bool Object::init()
	{
		bool res = true;
		for (auto& comp : components)
		{
			if(!comp->init()) res = false;
		}
		return res;
	}


	void Object::recreate_swapchain()
	{
		for (auto& comp : components)
		{
			comp->recreate_swapchain();
		}
	}
	void Object::draw(vk::CommandBuffer& cmd)
	{
		for (auto& comp : components)
		{
			comp->draw(cmd);
		}
	}
	void Object::update(float delta)
	{
		for (auto& comp : components)
		{
			comp->update(delta);
		}
	}
	void Object::late_update(float delta)
	{
		for (auto& comp : components)
		{
			comp->late_update(delta);
		}
	}
	void Object::clean_up()
	{
		for (auto& comp : components)
		{
			comp->clean_up();
		}
	}
	void Object::clean_up_pipeline()
	{
		for (auto& comp : components)
		{
			comp->clean_up_pipeline();
		}
	}
	Object::~Object()
	{
		for (auto& comp : components)
		{
			comp->set_enable(false);
			comp->on_destroy();
			comp->reset_object();
		}
		components.clear();
		locator.clear();
	}
}