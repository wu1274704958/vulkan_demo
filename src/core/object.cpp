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

	void Object::pre_draw(vk::CommandBuffer& cmd)
	{
		for (const auto& comp : components)
		{
			if (!comp->enable || comp->not_draw) continue;
			comp->pre_draw(cmd);
		}
	}


	void Object::draw(vk::CommandBuffer& cmd)
	{
		for (auto& comp : components)
		{
			if (!comp->enable || comp->not_draw) continue;
			comp->draw(cmd);
		}
	}
	void Object::after_draw(vk::CommandBuffer& cmd)
	{
		for (const auto& comp : components)
		{
			if (!comp->enable || comp->not_draw) continue;
			comp->after_draw(cmd);
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

	Object::~Object(){}

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

	void Object::attach_scene(const std::weak_ptr<Scene>& scene)
	{
		for (auto& comp : components)
		{
			comp->attach_scene(scene);
		}
	}

	void Object::detach_scene()
	{
		for (auto& comp : components)
		{
			comp->detach_scene();
		}
	}

	EngineState Object::engine_state()
	{
		return SampleRender::self_instance->engineState;
	}

	void Object::on_destroy()
	{
		auto running = engine_state() == EngineState::Running;
		for (const auto& comp : components)
		{
			if(running)
			{
				comp->clean_up_pipeline();
				comp->clean_up();
			}
			comp->set_enable(false);
			comp->on_destroy();
			comp->detach_object();
		}
		components.clear();
		locator.clear();
	}

	std::shared_ptr<Object> Object::clone() const
	{
		auto n = std::make_shared<Object>(*this);
		n->clone_components(*this);
		return n;
	}

	void Object::clone_components(const Object& oth)
	{
		for (auto& c : oth.locator)
		{
			size_t ty_id = c.first;
			auto comp = oth.components[c.second]->clone();
			add_comp(ty_id, comp);
		}
	}


	bool Object::has_comp(size_t id) const
	{
		return locator.contains(id);
	}

	bool Object::add_comp(size_t ty_id, std::shared_ptr<Component> comp)
	{
		if (has_comp(ty_id) || !comp) return false;
		comp->attach_object(weak_ptr());
		comp->awake();
		comp->set_enable(true);
		if (is_init) comp->init();
		if (components.empty() || components.back()->idx() <= comp->idx())
		{
			components.push_back(comp);
			locator.insert(std::make_pair(ty_id, components.size() - 1));
			return true;
		}
		else {
			for (int i = 0; i < components.size(); ++i)
			{
				if (comp->idx() <= components[i]->idx())
				{
					adjust_locat(i, 1);
					components.insert(components.begin() + i, comp);
					locator.insert(std::make_pair(ty_id, i));
					return true;
				}
			}
		}
		return false;
	}


	Object::Object(const Object& oth)
	{
		this->active = oth.active;
		this->name = oth.name;
	}



}