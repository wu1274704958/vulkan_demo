#include <core/component.hpp>
#include <core/object.hpp>

namespace vkd
{
	struct DefRenderPass : public Component
	{
		bool on_init() override;
		void on_clean_up() override;
		void awake() override;
		int64_t idx() override { return std::numeric_limits<int64_t>::min(); }
		void draw(vk::CommandBuffer& cmd) override;
		void after_draw(vk::CommandBuffer& cmd) override;
		const vk::RenderPass& getRenderPass() const;
	protected:
		vk::RenderPass render_pass;
	};
}

