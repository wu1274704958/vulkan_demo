#pragma once

#include <core/component.hpp>
#include <numeric>
#include <res_loader/data_mgr.hpp>
#include <glm/glm.hpp>


namespace vkd {
	struct Scene;
	struct Transform : public Component{
		friend Object;
		Transform();
		Transform(const Transform&);
		void awake() override{}
		bool on_init() override;
		void set_enable(bool v) override;
		void recreate_swapchain() override;
		void pre_draw(vk::CommandBuffer& cmd) override;
		void draw(vk::CommandBuffer& cmd) override;
		void after_draw(vk::CommandBuffer& cmd) override;
		void update(float delta) override;
		void late_update(float delta) override;
		void on_clean_up() override;
		void clean_up_pipeline() override;
		bool dispatchEvent(const evt::Event&) override;
		void attach_scene(const std::weak_ptr<Scene>& scene) override;
		void detach_scene() override;
		void on_destroy() override;
		int64_t idx() override { return std::numeric_limits<int64_t>::max(); }
		
		const glm::vec3& get_position()const;
		const glm::vec3& get_rotation()const;
		const glm::vec3& get_scale()   const;

		void set_position(glm::vec3 pos);
		void set_rotation(glm::vec3 rot);
		void set_scale(glm::vec3 scale);

		const glm::mat4& get_local_matrix();
		const glm::mat4& get_matrix();

		bool good_child_idx(int i) const;
		virtual bool add_child(std::shared_ptr<Transform> ch);
		std::shared_ptr<Transform> rm_child(int i);
		bool rm_child(Transform* ptr);
		bool rm_child(std::weak_ptr<Transform> ptr);
		bool has_parent() const ;
		std::weak_ptr<Transform> get_parent() const;
		std::weak_ptr<Transform> get_child(int i) const;
		std::weak_ptr<Transform> find_child(std::string_view name) const;
		bool matrix_dirty() const;
		virtual std::weak_ptr<Scene> get_scene();
		std::shared_ptr<Component> clone() const override;
	protected:
		void clone_childlren(const Transform& oth);
		void update_matrix();
	protected:
		glm::vec3 position,rotation,scale;
		glm::mat4 local_mat,mat;
		std::shared_ptr<Transform> parent;
		std::weak_ptr<Scene> scene;
		std::vector<std::shared_ptr<Transform>> childlren;
		std::vector<std::shared_ptr<Object>> objects;
		bool dirty:1 = true;
	};

}