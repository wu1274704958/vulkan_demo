#include  <comm_comp/renderpass.hpp>
#include <comm_comp/scene.hpp>
namespace vkd
{
	void DefRenderPass::awake()
	{
		m_render_pass = renderpass();
	}

	bool DefRenderPass::on_init()
	{
		const auto obj = object.lock();
		auto scene = obj->get_comp_raw<Scene>();
		scene->set_renderpass(m_render_pass);
		return true;
	}

	void DefRenderPass::pre_draw(vk::CommandBuffer& cmd)
	{
		cmd.beginRenderPass(render_pass_begin_info(),cnt);
	}

	void DefRenderPass::after_draw(vk::CommandBuffer& cmd)
	{
		cmd.endRenderPass();
	}

	vk::RenderPass DefRenderPass::getRenderPass() const
	{
		return m_render_pass;
	}



}

