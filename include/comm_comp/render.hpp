#pragma once
#include <core/component.hpp>
#include <core/object.hpp>
#include <comm_comp/scene.hpp>
#include <res_loader/data_vk_res.hpp>
#include <comm_comp/mesh.hpp>
#include <common.hpp>

namespace vkd
{
	struct DefRender : public Component
	{
		DefRender(){}
		DefRender(const DefRender&);
		void awake() override;
		bool on_init() override;
		void on_clean_up() override;
		void draw(vk::CommandBuffer& cmd) override;
		void late_update(float delta) override;
		void recreate_swapchain() override;
		int64_t idx()const  override{ return static_cast<int64_t>(CompIdx::Render); }
		std::shared_ptr<Component> clone() const override;
	protected:
		void update_vp();
		bool update_descriptor() const;

		struct Vp
		{
			glm::mat4 view;
			glm::mat4 proj;
		} vp;
		std::shared_ptr<gld::vkd::VkdBuffer> vp_buf;
		std::weak_ptr<MeshInterface> mesh;
	};

	struct DefRenderInstance : public DefRender
	{
		DefRenderInstance(){}
		DefRenderInstance(const DefRenderInstance&);
		bool on_init() override;
		void draw(vk::CommandBuffer& cmd) override;
		std::shared_ptr<Component> clone() const override;
	protected:
		std::weak_ptr<MeshInstanceInterface> meshInstance;
	};

	struct RenderNoIndex : public DefRender
	{
		RenderNoIndex() {}
		RenderNoIndex(const RenderNoIndex&);
		void draw(vk::CommandBuffer& cmd) override;
		std::shared_ptr<Component> clone() const override;
	protected:
	};
}
