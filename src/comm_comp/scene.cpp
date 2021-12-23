#include <comm_comp/scene.hpp>
#include <comm_comp/showcase.hpp>
#include <comm_comp/sky_box.hpp>


namespace vkd
{
	Scene::Scene(){}
	Scene::Scene(const Scene& oth)
	{
	}


	std::weak_ptr<Scene> Scene::get_scene()
	{
		return std::dynamic_pointer_cast<Scene>(shared_from_this());
	}

	std::shared_ptr<Component> Scene::clone() const
	{
		auto n = std::make_shared<Scene>(*this);
		n->clone_childlren(*this);
		return n;
	}

	std::shared_ptr<Component> Scene::iterat(std::vector<std::weak_ptr<Component>>& cs, std::function<bool(std::shared_ptr<Component>&)> f)
	{
		for (int i = cs.size() - 1; i >= 0; --i)
		{
			auto comp = cs[i].lock();
			if (!comp || !(comp->get_object()))
			{
				cs.erase(cs.begin() + i);
				continue;
			}
			auto trans = comp->get_object()->get_comp_dyn<Transform>().lock();
			if (!trans || trans->get_scene().expired() || trans->get_scene().lock().get() != this)
			{
				cs.erase(cs.begin() + i);
				continue;
			}

			if (comp->is_enable())
			{
				if(f(comp))
					return comp;
			}
		}
		return nullptr;
	}


	bool Scene::add_bind_comp(std::type_index idx, std::shared_ptr<Component> comp)
	{
		if (bind_comps.contains(idx))
		{
			auto& cs = bind_comps[idx];
			auto find = iterat(cs,[&comp](std::shared_ptr<Component>& c)
			{
				return c.get() == comp.get();	
			});
			if(find) return false;
			cs.push_back(comp);
		}else
		{
			bind_comps.insert(std::make_pair(idx,std::vector<std::weak_ptr<Component>>()));
			bind_comps[idx].push_back(comp);
		}
		return true;
	}




}
