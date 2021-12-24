#pragma once

#include <memory>
#include <vector>
#include <core/component.hpp>
#include <event/event.hpp>
#include <common.hpp>
#include <comm_comp/transform.hpp>

namespace vkd {
	struct Transform;
	struct Scene;

	struct Object : public std::enable_shared_from_this<Object>,public evt::EventDispatcher,public Clone<Object> {
		friend Component;
		friend Transform;
		Object(){}
		Object(std::string name) : name(name){}
		Object(const Object&);
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

		bool has_comp(size_t id) const;

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
			if(add_comp(ty_id,std::dynamic_pointer_cast<Component>(comp)))
				return comp;
			return {};
		}
		bool add_comp(size_t id, std::shared_ptr<Component> comp);

		template<typename T>
		requires requires() {
			requires std::is_base_of_v<Component, T>;
		}
		bool add_comp(std::shared_ptr<T> comp)
		{
			size_t ty_id = typeid(T).hash_code();
			return add_comp(ty_id,comp);
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
			return {};//get_comp_by_base<T,T>();
		}
		/*template<typename T, typename CT>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		std::weak_ptr<T> get_comp_by_base()
		{
			return {};
		}
		template<typename T,typename CT>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
			CT::__BASE_TYPE;
		}
		std::weak_ptr<T> get_comp_by_base()
		{
			size_t ty_id = typeid(CT::__BASE_TYPE).hash_code();
			if 
			{
				int idx = locator[ty_id];
				return std::dynamic_pointer_cast<T>(components[idx]);
			}
			return get_comp_by_base<T,CT::__BASE_TYPE>();
		}*/
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
				auto c = components[idx];
				components.erase(components.begin() + idx);
				if (idx < components.size())
					adjust_locat(idx,-1);
				locator.erase(ty_id);
				if(auto transform = get_comp_raw<Transform>();transform && !transform->get_scene().expired())
					c->detach_scene();
				if(engine_state() == EngineState::Running && c->is_init)
				{
					c->clean_up_pipeline();
					c->clean_up();
				}
				c->set_enable(false);
				c->on_destroy();
				c->detach_object();
			}
		}

		template<typename T>
		requires requires()
		{
			requires std::is_base_of_v<Component, T>;
		}
		std::weak_ptr<T> get_comp_dyn()
		{
			for(auto& a : components)
			{
				std::shared_ptr<T> ptr = std::dynamic_pointer_cast<T>(a);
				if(ptr) return ptr;
			}
			return {};
		}
		uint32_t component_count();
		void set_active(bool v);
		bool is_active();

		bool init();
		void recreate_swapchain();
		void pre_draw(vk::CommandBuffer& cmd) ;
		void draw(vk::CommandBuffer& cmd);
		void after_draw(vk::CommandBuffer& cmd);
		void update(float delta);
		void late_update(float delta);
		void clean_up();
		void clean_up_pipeline();
		std::shared_ptr<Object> clone() const override;
		~Object();
		bool dispatchEvent(const evt::Event&) override;
		void attach_scene(const std::weak_ptr<Scene>&);
		void detach_scene();
		void on_destroy();
		static EngineState engine_state();
	protected:
		void adjust_locat(size_t i,int offset);
		bool good_comp_idx(int idx);
		std::shared_ptr<Object> ptr();
		std::weak_ptr<Object> weak_ptr();
		void clone_components(const Object&);

		std::vector<std::shared_ptr<Component>> components;
		std::unordered_map<size_t,uint32_t> locator;
		std::string name;
		bool active : 1 = true;
		bool is_init : 1 = false;
	};
}