#include "core/object.hpp"
#include <sample/render.hpp>
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
				{
					if(v){ 
						if(is_init && !comp->is_init)
							comp->init();
						comp->on_enable(); 
					}else
						comp->on_disable();
				}
			}
			if(v && engine_state() == EngineState::Running && !is_init)  
				init();
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
			if(!comp->enable) continue;
			if(!comp->init()) res = false;
		}
		is_init = true;
		return res;
	}


	void Object::recreate_swapchain()
	{
		for (auto& comp : components)
		{
			if (!comp->enable) continue;
			comp->recreate_swapchain();
		}
	}
	void Object::draw(vk::CommandBuffer& cmd)
	{
		for (auto& comp : components)
		{
			if (!comp->enable) continue;
			comp->draw(cmd);
		}
	}
	void Object::update(float delta)
	{
		for (auto& comp : components)
		{
			if(comp->enable && comp->ever_tick)
				comp->update(delta);
		}
	}
	void Object::late_update(float delta)
	{
		for (auto& comp : components)
		{
			if (comp->enable && comp->ever_tick)
				comp->late_update(delta);
		}
	}
	void Object::clean_up()
	{
		for (auto& comp : components)
		{
			if(comp->is_init)
				comp->clean_up();
		}
		is_init = false;
	}
	void Object::clean_up_pipeline()
	{
		for (auto& comp : components)
		{
			if (comp->is_init)
				comp->clean_up_pipeline();
		}
	}
	Object::~Object()
	{
		for (auto& comp : components)
		{
			comp->set_enable(false);
			comp->on_destroy();
			comp->detach_object();
		}
		components.clear();
		locator.clear();
	}

	bool Object::dispatchEvent(const evt::Event& e)
	{
		for (auto& comp : components)
		{
			if(!comp->enable) continue;
			if(comp->dispatchEvent(e))
				return true;
		}
		return false;
	}

	void Object::attach_scene()
	{
		for (auto& comp : components)
		{
			comp->attach_scene();
		}
	}

	void Object::detach_scene()
	{
		for (auto& comp : components)
		{
			comp->attach_scene();
		}
	}

	EngineState Object::engine_state()
	{
		return SampleRender::self_instance->engineState;
	}
}