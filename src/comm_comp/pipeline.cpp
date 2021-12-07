#include <comm_comp/pipeline.hpp>
#include <comm_comp/scene.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>

namespace vkd {
	void PipelineComp::draw(vk::CommandBuffer& cmd)
	{
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
		const std::vector<vk::DescriptorSet>& sets = descSets;
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipelineLayout, 0,sets, {});
		if (!get_object()->has_comp<ViewportScissor>())
		{
			 auto surfaceExtent = surface_extent();
			 vk::Viewport viewport(0, 0, (float)surfaceExtent.width, (float)surfaceExtent.height, 0.0f, 1.0f);
			 cmd.setViewport(0, viewport);
			 vk::Rect2D scissor({}, surfaceExtent);
			 cmd.setScissor(0, scissor);
		}
	}

	bool PipelineComp::on_init()
	{
		if (vertexPath.empty() || fragPath.empty()) return false;
		const auto obj = object.lock();
		vk::RenderPass render_pass = obj->get_comp_raw<Transform>()->get_scene().lock()->get_renderpass();
		pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device(), render_pass, surface_extent(),
			vertexPath, fragPath,maxSetSize,instanceSet,bindingSplit);
		if (!pipeline) return false;
		descSets = pipeline->allocDescriptorSets();
		return true;
	}

	void PipelineComp::recreate_swapchain()
	{
		on_init();
	}

	void PipelineComp::on_clean_up() {
		
	}
	void PipelineComp::clean_up_pipeline() {
		descSets.cleanup(device());
		pipeline.reset();
	}
	const gld::vkd::PipelineData const* PipelineComp::get_pipeline() const
	{
		if (pipeline)
			return pipeline.get();
		else 
			return nullptr;
	}
	const std::vector<vk::DescriptorSet>& PipelineComp::get_descriptorsets() const
	{
		return descSets;
	}
}