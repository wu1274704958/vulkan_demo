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
		vk::DescriptorPool descriptorPool;
		std::vector<vk::ShaderModule> shaderModules;

		~PipelineData() {
			if(!device) return;
			if (pipeline) device.destroyPipeline(pipeline);
			for(auto r : shaderModules)
			{
				if (r) device.destroyShaderModule(r);
			}
			if(descriptorPool) device.destroyDescriptorPool(descriptorPool);
			if (pipelineLayout) device.destroyPipelineLayout(pipelineLayout);
			if(setLayout) device.destroyDescriptorSetLayout(setLayout);

		}
	};
	template<typename ...Args>
	struct LoadPipelineSimple {
		using RetTy = std::shared_ptr<PipelineData>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&,const std::string&);
		static std::string key_from_args(vk::Device,vk::RenderPass,const vk::Extent2D&,
			const std::string&,const std::string&,const std::unordered_set<uint32_t>& ins_set = {},std::function<void(vk::GraphicsPipelineCreateInfo)> on = {},uint32_t maxPoolSize = 1);
		static RealRetTy load(vk::Device, vk::RenderPass, const vk::Extent2D&,
			std::string, std::string, std::unordered_set<uint32_t> ins_set = {}, std::function<void(vk::GraphicsPipelineCreateInfo)> on = {}, uint32_t maxPoolSize = 1);
	};
}