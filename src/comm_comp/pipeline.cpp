#include <comm_comp/pipeline.hpp>
#include <comm_comp/renderpass.hpp>
#include <comm_comp/scene.hpp>
#include <core/object.hpp>
#include <comm_comp/transform.hpp>

namespace vkd {


	void ViewportScissor::reset(glm::vec4 vp, glm::vec4 sc, float minDepth , float maxDepth)
	{
		auto extent = surface_extent();
		viewport.x = vp.x * extent.width;
		viewport.y = vp.y * extent.height;
		viewport.width = vp.z * extent.width;
		viewport.height = vp.w * extent.height;
		scissor.offset = vk::Offset2D{ static_cast<int32_t>(sc.x * extent.width),static_cast<int32_t>(sc.y * extent.height) };
		scissor.extent = vk::Extent2D{ static_cast<uint32_t>(sc.z * extent.width),static_cast<uint32_t>(sc.w * extent.height) };
	}

	void ViewportScissor::draw(vk::CommandBuffer& cmd)
	{
		cmd.setViewport(0, viewport);
		cmd.setScissor(0, scissor);
	}


	void ViewportScissor::reset()
	{
		auto extent = surface_extent();
		viewport.x = viewport_ratio.x * extent.width;
		viewport.y = viewport_ratio.y * extent.height;
		viewport.width = viewport_ratio.z * extent.width;
		viewport.height = viewport_ratio.w * extent.height;
		scissor.offset = vk::Offset2D{ static_cast<int32_t>(scissor_ratio.x * extent.width),static_cast<int32_t>(scissor_ratio.y * extent.height) };
		scissor.extent = vk::Extent2D{ static_cast<uint32_t>(scissor_ratio.z * extent.width),static_cast<uint32_t>(scissor_ratio.w * extent.height) };
	}

	void ViewportScissor::recreate_swapchain()
	{
		reset();
	}

	std::shared_ptr<Component> ViewportScissor::clone() const
	{
		return std::make_shared<ViewportScissor>(*this);
	}



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
		const auto trans = obj->get_comp_dyn<Transform>().lock();
		if(!trans) return false;
		auto defRp = trans->get_scene().lock()->get_bind_comp<DefRenderPass>();
		if(!defRp) return false;
		vk::RenderPass render_pass = defRp->getRenderPass();
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
	std::shared_ptr<Component> PipelineComp::clone() const
	{
		return std::make_shared<PipelineComp>(*this);
	}
	PipelineComp::PipelineComp(const PipelineComp& oth)
	{
		this->vertexPath = oth.vertexPath;
		this->fragPath = oth.fragPath;
		this->maxSetSize = oth.maxSetSize;
		this->instanceSet = oth.instanceSet;
		this->bindingSplit = oth.bindingSplit;
		this->pipeline = oth.pipeline;
		if(pipeline)
			this->descSets = this->pipeline->allocDescriptorSets();
	}


}