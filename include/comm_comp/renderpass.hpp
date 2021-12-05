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
		vk::RenderPass m_render_pass;
		vk::SubpassContents cnt;
	};
}

