#pragma once

#include <memory>
#include <vector>
#include <core/component.hpp>
#include <event/event.hpp>
#include <common.hpp>

namespace vkd {
	struct Transform;

	struct Object : public std::enable_shared_from_this<Object>,public evt::EventDispatcher {
		friend Component;
		friend Transform;
		Object(){}
		Object(std::string name) : name(name){}
		template<typename T>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		bool has_comp()
		{
			size_t ty_id = typeid(T).hash_code();
			return locator.contains(ty_id);
		}

		template<typename T,typename ... Args>
		requires requires(Args&& ...args){
			new T(std::forward<Args>(args)...);
			requires std::is_base_of_v<Component,T>;
		}
		std::weak_ptr<T> add_comp(Args&& ...args)
		{
			if(has_comp<T>()) return {};
			size_t ty_id = typeid(T).hash_code();
			auto comp = std::make_shared<T>(std::forward<Args>(args)...);
			comp->attach_object(weak_ptr());
			comp->awake();
			comp->set_enable(true);
			if(is_init) comp->init();
			if (components.empty() || components.back()->idx() <= comp->idx())
			{
				components.push_back(std::move(comp));
				locator.insert(std::make_pair(ty_id,components.size() - 1));
				return comp;
			}
			else {
				for (int i = 0; i < components.size(); ++i)
				{
					if (comp->idx() <= components[i]->idx())
					{
						adjust_locat(i,1);
						components.insert(components.begin() + i, std::move(comp));
						locator.insert(std::make_pair(ty_id, i));
						return comp;
					}
				}
			}
			return {};
		}
		template<typename T>
			requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		std::weak_ptr<T> get_comp()
		{
			size_t ty_id = typeid(T).hash_code();
			if (locator.contains(ty_id))
			{
				int idx = locator[ty_id];
				return std::dynamic_pointer_cast<T>(components[idx]);
			}
			return {};
		}
		template<typename T>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		T* get_comp_raw()
		{
			size_t ty_id = typeid(T).hash_code();
			if (locator.contains(ty_id))
			{
				int idx = locator[ty_id];
				return dynamic_cast<T*>(components[idx].get());

			}
			return {};
		}
		template<typename T>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		void destroy_comp()
		{
			size_t ty_id = typeid(T).hash_code();
			if (locator.contains(ty_id))
			{
				int idx = locator[ty_id];
				auto c = components.erase(components.begin() + idx);
				if (idx < components.size())
					adjust_locat(idx,-1);
				locator.erase(ty_id);
				(*c)->set_enable(false);
				(*c)->on_destroy();
				(*c)->detach_object();
			}
		}
		uint32_t component_count();
		void set_active(bool v);
		bool is_active();

		bool init();
		void recreate_swapchain();
		void draw(vk::CommandBuffer& cmd);
		void update(float delta);
		void late_update(float delta);
		void clean_up();
		void clean_up_pipeline();
		~Object();
		bool dispatchEvent(const evt::Event&) override;
		void attach_scene();
		void detach_scene();
		
		static EngineState engine_state();
	protected:
		void adjust_locat(size_t i,int offset);
		bool good_comp_idx(int idx);
		std::shared_ptr<Object> ptr();
		std::weak_ptr<Object> weak_ptr();

		std::vector<std::shared_ptr<Component>> components;
		std::unordered_map<size_t,uint32_t> locator;
		std::string name;
		bool active : 1 = false;
		bool is_init : 1 = false;
	};
}