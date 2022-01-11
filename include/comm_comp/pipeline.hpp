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
		ViewportScissor(glm::vec4 vp,glm::vec4 sc,float minDepth = 0.0f,float maxDepth = 1.0f) : viewport_ratio(vp),scissor_ratio(sc)
		{
			reset(vp,sc,minDepth,maxDepth);
		}
		void reset(glm::vec4 vp, glm::vec4 sc, float minDepth = 0.0f, float maxDepth = 1.0f);
		void reset();
		virtual bool on_init() override {return true;}
		virtual void awake() override {}
		virtual void on_enable() override{}
		virtual void on_disable() override{}
		virtual void recreate_swapchain() override;
		virtual void draw(vk::CommandBuffer& cmd) override;
		virtual void update(float delta) override{}
		virtual void late_update(float delta) override{}
		virtual void on_clean_up() override{}
		virtual void clean_up_pipeline() override{}
		int64_t idx()const  override { return static_cast<int64_t>(CompIdx::ViewportScissor); }
		std::shared_ptr<Component> clone() const override;
		glm::vec4 viewport_ratio,scissor_ratio;
		vk::Viewport viewport;
		vk::Rect2D scissor;
	};


	struct PipelineComp : public Component
	{
		PipelineComp(std::string vert,std::string frag, uint32_t maxSetSize = 1) :
			vertexPath(vert),fragPath(frag),maxSetSize(maxSetSize)
			{}
		PipelineComp(std::string vert, std::string frag,
			uint32_t maxSetSize,
			std::unordered_set<uint32_t> instanceSet = {},
			std::vector<uint32_t> bindingSplit = {}) :
				vertexPath(vert), fragPath(frag) ,maxSetSize(maxSetSize),instanceSet(instanceSet),bindingSplit(bindingSplit)
		{}
		PipelineComp(const PipelineComp&);
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
		int64_t idx() const  override { return static_cast<int64_t>(CompIdx::Pipeline); }
		const gld::vkd::PipelineData const* get_pipeline() const;
		const std::vector<vk::DescriptorSet>& get_descriptorsets() const;
		std::shared_ptr<Component> clone() const override;
		std::string vertexPath, fragPath;
		uint32_t maxSetSize = 1;
		std::unordered_set<uint32_t> instanceSet;
		std::vector<uint32_t> bindingSplit;
	protected:
		std::shared_ptr<gld::vkd::PipelineData> pipeline;
		gld::vkd::DescriptorSets descSets;
	};
}