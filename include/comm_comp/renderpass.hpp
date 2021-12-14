#include <core/component.hpp>
#include <core/object.hpp>

namespace vkd
{
	struct DefRenderPass : public Component
	{
		DefRenderPass(vk::SubpassContents cnt = vk::SubpassContents::eInline) : cnt(cnt) {}
		bool on_init() override;
		void on_clean_up() override{};
		void awake() override;
		int64_t idx() override { return 0; }
		void pre_draw(vk::CommandBuffer& cmd) override;
		void after_draw(vk::CommandBuffer& cmd) override;
		vk::RenderPass getRenderPass() const;
		void recreate_swapchain() override;
		std::shared_ptr<Component> clone() const override;
	protected:
		virtual vk::RenderPass create_renderpass();
		virtual void renderpass_begin(vk::CommandBuffer& cmd,vk::SubpassContents cnt);
		vk::RenderPass m_render_pass;
		vk::SubpassContents cnt;
	};

	struct OnlyDepthRenderPass : public DefRenderPass
	{
		OnlyDepthRenderPass() : DefRenderPass(){}
		OnlyDepthRenderPass(const OnlyDepthRenderPass&);
		void awake() override;
		void clean_up_pipeline() override;
		vk::ImageView get_image_view() const;
		vk::ImageLayout get_image_layout() const;
		std::shared_ptr<Component> clone() const override;
	protected:
		void create_depth_attachment();
		vk::RenderPass create_renderpass() override;
		void renderpass_begin(vk::CommandBuffer& cmd, vk::SubpassContents cnt) override;
		vk::Framebuffer framebuffer;
		vk::Image depth;
		vk::ImageView view;
		vk::DeviceMemory mem;
	};
}

