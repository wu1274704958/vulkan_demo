#pragma once
#include <core/component.hpp>
#include <core/object.hpp>
#include <typeindex>

namespace vkd
{
	struct Camera;
	struct SkyBox;

	struct Scene : public Transform
	{
		Scene();
		Scene(const Scene&);
		std::weak_ptr<Scene> get_scene() override;

		template<typename T>
		requires requires() {
			requires std::is_base_of_v<Component, T>;
		}
		bool add_bind_comp(std::shared_ptr<T> comp)
		{
			std::type_index idx = std::type_index(typeid(T));
			return add_bind_comp(idx, comp);
		}

		template<typename T>
		requires requires() {
			requires std::is_base_of_v<Component, T>;
		}
		std::shared_ptr<T> get_bind_comp()
		{
			std::type_index idx = std::type_index(typeid(T));
			if(!bind_comps.contains(idx)) return {};
			auto& cs = bind_comps[idx];
			
			return std::dynamic_pointer_cast<T>(iterat(cs, [](std::shared_ptr<Component>& c)
			{
				return true;
			}));
		}
		std::shared_ptr<Component> iterat(std::vector<std::weak_ptr<Component>>& cs,std::function<bool(std::shared_ptr<Component>&)> f);
		bool add_bind_comp(std::type_index idx,std::shared_ptr<Component> comp);
		
		std::shared_ptr<Component> clone() const override;
	protected:
		std::unordered_map<std::type_index, std::vector<std::weak_ptr<Component>>> bind_comps;
	};	
}
