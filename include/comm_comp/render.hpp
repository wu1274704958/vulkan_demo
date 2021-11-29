#pragma once
#include <core/component.hpp>
#include <core/object.hpp>
#include <comm_comp/scene.hpp>
#include <res_loader/data_vk_res.hpp>
#include <comm_comp/mesh.hpp>

namespace vkd
{
	struct DefRender : public Component
	{
		void awake() override;
		bool on_init() override;
		void on_clean_up() override;
		void draw(vk::CommandBuffer& cmd) override;
		void late_update(float delta) override;
		void recreate_swapchain() override;
		int64_t idx() override{ return std::numeric_limits<int64_t>::max() - 1; }
	protected:
		void update_vp();
		bool update_descriptor() const;

		struct Vp
		{
			glm::mat4 view;
			glm::mat4 proj;
		} vp;
		std::shared_ptr<gld::vkd::VkdBuffer> vp_buf;
		std::weak_ptr<MeshComp> mesh;
	};
}
