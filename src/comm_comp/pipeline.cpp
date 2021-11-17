#include <comm_comp/pipeline.hpp>
#include <core/object.hpp>

namespace vkd {
	void PipelineComp::draw(vk::CommandBuffer& cmd)
	{
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipelineLayout, 0, descSets, {});
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
		pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device(), renderpass(), surface_extent(),
			vertexPath, fragPath, maxPoolSize, instance_set, on_create_pipeline);
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
		pipeline.reset();
	}
}