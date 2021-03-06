#include <comm_comp/transform.hpp>
#include <core/object.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkd {
	Transform::Transform() : Component(),position(0.0f),rotation(0.0f),
		scale(1.0f),mat(1.0f),parent(nullptr)
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
			if(const auto obj = ch->object.lock();obj && obj->active)
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
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->recreate_swapchain();
			}
		}
	}

	void Transform::pre_draw(vk::CommandBuffer& cmd)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->pre_draw(cmd);
			}
		}
	}


	void Transform::draw(vk::CommandBuffer& cmd)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->draw(cmd);
			}
		}
	}

	void Transform::after_draw(vk::CommandBuffer& cmd)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->after_draw(cmd);
			}
		}
	}


	void Transform::update(float delta)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->update(delta);
			}
		}
	}

	void Transform::late_update(float delta)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				obj->late_update(delta);
			}
		}
	}

	void Transform::on_clean_up()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->is_init)
			{
				obj->clean_up();
			}
		}
	}

	void Transform::clean_up_pipeline()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->is_init)
			{
				obj->clean_up_pipeline();
			}
		}
	}

	bool Transform::dispatchEvent(const evt::Event& e)
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj && obj->active)
			{
				if(obj->dispatchEvent(e)) return true;
			}
		}
		return false;
	}

	void Transform::attach_scene(const std::weak_ptr<Scene>& scene)
	{
		this->scene = scene;
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock();obj)
			{
				obj->attach_scene(scene);
			}
		}
	}

	void Transform::detach_scene()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->detach_scene();
			}
		}
	}

	void Transform::on_destroy()
	{
		for (const auto& ch : childlren)
		{
			if (const auto obj = ch->object.lock(); obj)
			{
				obj->on_destroy();
			}
		}
	}

	const glm::vec3& Transform::get_position()const { return position; }
	const glm::vec3& Transform::get_rotation()const { return rotation; }
	const glm::vec3& Transform::get_scale()   const { return scale; }
	void Transform::set_position(glm::vec3 pos) { position = pos;dirty = true; }
	void Transform::set_rotation(glm::vec3 rot) { rotation = rot; dirty = true; }
	void Transform::set_scale(glm::vec3 scale) { this->scale = scale; dirty = true; }

	const glm::mat4& Transform::get_local_matrix()
	{
		if(dirty) update_matrix();
		return local_mat;
	}

	bool Transform::matrix_dirty() const
	{
		return dirty || (parent ? parent->dirty : false);
	}

	const glm::mat4& Transform::get_matrix()
	{
		if(matrix_dirty())
		{
			if (dirty) update_matrix();
			if (parent) 
				mat = get_parent().lock()->get_matrix() * local_mat;
			else
				mat = local_mat;
			return mat;
		}else
			return mat;
	}

	void Transform::update_matrix()
	{
		local_mat = glm::mat4(1.0f);
		local_mat = glm::translate(local_mat, position);

		local_mat = glm::scale(local_mat, scale);

		local_mat = glm::rotate(local_mat, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
		local_mat = glm::rotate(local_mat, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
		local_mat = glm::rotate(local_mat, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));

		dirty = false;
	}

	bool Transform::good_child_idx(int i) const
	{
		return i >= 0 && i < childlren.size();
	}

	bool Transform::add_child(std::shared_ptr<Transform> ch)
	{
		if (!ch || ch.get() == this || ch == parent) return false;
		const auto obj = ch->object.lock();
		if (!obj) return false;
		for(const auto& c : childlren)
		{
			if(c == ch) return false;
		}
		if(ch->parent)
			ch->parent->rm_child(ch.get());
		ch->parent = std::dynamic_pointer_cast<Transform>(shared_from_this());
		ch->scene = get_scene();
		childlren.push_back(ch);
		objects.push_back(obj);
		if(!(ch->scene.expired()))
			obj->attach_scene(ch->scene);
		if(is_init && !obj->is_init && obj->active)
			obj->init();
		
		return true;
	}

	bool Transform::rm_child(Transform* ptr)
	{
		size_t k = -1;
		for(size_t i = 0;i < childlren.size();++i)
		{
			if(childlren[i].get() == ptr)
			{
				k = i;break;
			}
		}
		if(k >= 0 && k < childlren.size())
		{
			auto it = childlren[k];
			{
				if(!(it->scene.expired()))
				{
					if(const auto obj = it->object.lock();obj)
						obj->detach_scene();
				}
				it->parent.reset();
				it->scene.reset();
				return true;
			}
		}
		return false;
	}

	bool Transform::rm_child(std::weak_ptr<Transform> ptr)
	{
		if(const auto p = ptr.lock(); p)
			return rm_child(p.get());
		return false;
	}

	std::shared_ptr<Transform> Transform::rm_child(int i)
	{
		if(good_child_idx(i))
		{
			auto it = childlren[i];
			childlren.erase(childlren.begin() + i);
			objects.erase(objects.begin() + i);
			if (!(it->scene.expired()))
			{
				if (const auto obj = it->object.lock(); obj)
					obj->detach_scene();
			}
			it->parent.reset();
			it->scene.reset();
			return it;
		}
		return nullptr;
	}

	bool Transform::has_parent() const
	{
		return static_cast<bool>(parent);
	}

	std::weak_ptr<Transform> Transform::get_parent() const
	{
		return parent;
	}

	std::weak_ptr<Transform> Transform::get_child(int i) const
	{
		if(good_child_idx(i))
			return childlren[i];
		return {};
	}

	std::weak_ptr<Transform> Transform::find_child(std::string_view name_sv) const
	{
		if (name_sv.empty()) return {};

		const Transform* curr = this;

		while (curr && !name_sv.empty())
		{
			size_t idx = name_sv.find_first_of('/');
			std::string_view pre, after;
			bool has_child = false;
			if (idx == std::string::npos)
				pre = name_sv;
			else
			{
				pre = name_sv.substr(0, idx);
				after = name_sv.substr(idx + 1);
				has_child = true;
			}
			bool find = false;
			for (const auto& ch : curr->childlren)
			{
				if (const auto obj = ch->object.lock(); obj && obj->name == pre)
				{
					find = true;
					if (!has_child)
						return ch;
					else
					{
						curr = ch.get();
						name_sv = after;
					}
					break;
				}
			}
			if(!find) return {};
		}
		return {};
	}

	std::weak_ptr<Scene> Transform::get_scene()
	{
		return scene;
	}

	std::shared_ptr<Component> Transform::clone() const
	{
		auto n = std::make_shared<Transform>(*this);
		n->clone_childlren(*this);
		return n;
	}

	Transform::Transform(const Transform& oth)
	{
		this->position = oth.position;
		this->rotation = oth.rotation;
		this->scale = oth.scale;
		this->local_mat = oth.local_mat;
		this->mat = oth.mat;
	}

	void Transform::clone_childlren(const Transform& oth)
	{
		for (auto& ch : oth.objects)
		{
			auto child = ch->clone();
			auto trans = child->get_comp<Transform>();
			add_child(trans.lock());
		}
	}



}
