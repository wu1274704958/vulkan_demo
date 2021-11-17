#pragma once
#include <core/component.hpp>
#include <numeric>
#include <unordered_set>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>

namespace vkd {
	struct PipelineComp : public Component
	{
		PipelineComp(std::string vert,std::string frag, uint32_t maxPoolSize = 1) : 
			vertexPath(vert),fragPath(frag),maxPoolSize(maxPoolSize)
			{}
		virtual void awake() override{}
		virtual bool on_init() override
		{
			if(vertexPath.empty() || fragPath.empty()) return false;
			pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device(),renderpass(),surface_extent(),
				vertexPath,fragPath,maxPoolSize,instance_set, on_create_pipeline);
			if(!pipeline) return false;
			return true;
		}
		virtual void on_enable() override
		{
		}
		virtual void on_disable() override
		{
		}
		virtual void recreate_swapchain() override
		{
		}
		virtual void draw(vk::CommandBuffer& cmd) override
		{
		}
		virtual void update(float delta) override
		{
		}
		virtual void late_update(float delta) override
		{
		}
		virtual void on_clean_up() override
		{
		}
		virtual void clean_up_pipeline() override
		{
		}
		virtual int64_t idx() override {
			return -999999999;
		}
		std::string vertexPath, fragPath;
		uint32_t maxPoolSize = 1;
		std::unordered_set<uint32_t> instance_set;
		std::function<void(vk::GraphicsPipelineCreateInfo)> on_create_pipeline;
	protected:
		std::shared_ptr<gld::vkd::PipelineData> pipeline;
	};
}