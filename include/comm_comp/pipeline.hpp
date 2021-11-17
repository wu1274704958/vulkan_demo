#pragma once
#include <core/component.hpp>
#include <numeric>
#include <unordered_set>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>


namespace vkd {

	struct ViewportScissor : public Component
	{
		ViewportScissor(): 
			viewport(0.0f, 0.0f, (float)surface_extent().width, (float)surface_extent().height, 0.0f, 1.0f),
			scissor({}, surface_extent()){}
		ViewportScissor(glm::vec4 vp,glm::vec4 sc,float minDepth = 0.0f,float maxDepth = 1.0f){
			auto w = surface_extent().width;
			auto h = surface_extent().height;
			viewport = vk::Viewport(vp.x * w, vp.y * h, vp.z * w, vp.w * h, minDepth,maxDepth);
			scissor = vk::Rect2D({static_cast<int32_t>(sc.x * w),static_cast<int32_t>(sc.y * h)},{static_cast<uint32_t>(sc.z * w),static_cast<uint32_t>(sc.w * h)});
		}
		virtual bool on_init() override {}
		virtual void awake() override {}
		virtual void on_enable() override{}
		virtual void on_disable() override{}
		virtual void recreate_swapchain() override{}
		virtual void draw(vk::CommandBuffer& cmd) override
		{
			cmd.setViewport(0, viewport);
			cmd.setScissor(0, scissor);
		}
		virtual void update(float delta) override{}
		virtual void late_update(float delta) override{}
		virtual void on_clean_up() override{}
		virtual void clean_up_pipeline() override{}
		virtual int64_t idx() override {
			return -99999999;
		}
		vk::Viewport viewport;
		vk::Rect2D scissor;
	};


	struct PipelineComp : public Component
	{
		PipelineComp(std::string vert,std::string frag, uint32_t maxPoolSize = 1) : 
			vertexPath(vert),fragPath(frag),maxPoolSize(maxPoolSize)
			{}
		virtual void awake() override{}
		virtual bool on_init() override;
		virtual void on_enable() override{}
		virtual void on_disable() override{}
		virtual void recreate_swapchain() override;
		virtual void draw(vk::CommandBuffer& cmd) override;
		virtual void update(float delta) override{}
		virtual void late_update(float delta) override{}
		virtual void on_clean_up() override;
		virtual void clean_up_pipeline() override;
		virtual int64_t idx() override {
			return -999999999;
		}
		std::string vertexPath, fragPath;
		uint32_t maxPoolSize = 99999;
		std::unordered_set<uint32_t> instance_set;
		std::vector<vk::DescriptorSet> descSets;
		std::function<void(vk::GraphicsPipelineCreateInfo)> on_create_pipeline;
	protected:
		std::shared_ptr<gld::vkd::PipelineData> pipeline;
	};
}