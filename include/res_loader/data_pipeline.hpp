#pragma once
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <unordered_set>

namespace gld::vkd {

	struct PipelineData
	{
		vk::Device device;
		vk::DescriptorSetLayout setLayout;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
		std::vector<vk::ShaderModule> shaderModules;

		~PipelineData() {
			if(!device) return;
			if (pipeline) device.destroyPipeline(pipeline);
			for(auto r : shaderModules)
			{
				if (r) device.destroyShaderModule(r);
			}
			if (pipelineLayout) device.destroyPipelineLayout(pipelineLayout);
			if(setLayout) device.destroyDescriptorSetLayout(setLayout);

		}
	};

	struct LoadPipelineSimple {
		using RetTy = std::shared_ptr<PipelineData>;
		using ArgsTy = std::tuple<vk::Device,std::string,std::string,std::unordered_set<uint32_t>,vk::Extent2D,vk::RenderPass>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const ArgsTy& args);
		static RealRetTy load(ArgsTy args);
	};
}