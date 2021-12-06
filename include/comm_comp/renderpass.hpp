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
	protected:
		virtual vk::RenderPass create_renderpass();
		virtual vk::RenderPassBeginInfo renderpass_begin();
		vk::RenderPass m_render_pass;
		vk::SubpassContents cnt;
	};

	struct OnlyDepthRenderPass : public DefRenderPass
	{
		void awake() override;
		void clean_up_pipeline() override;
		void recreate_swapchain() override;
		vk::ImageView get_image_view() const;
		vk::ImageLayout get_image_layout() const;
	protected:
		void create_depth_attachment();
		vk::RenderPass create_renderpass() override;
		vk::RenderPassBeginInfo renderpass_begin() override;
		vk::Framebuffer framebuffer;
		vk::Image depth;
		vk::ImageView view;
		vk::DeviceMemory mem;
	};
}

