#pragma once
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <unordered_set>

namespace gld::vkd {

	struct PipelineData
	{
		vk::Device device;
		std::vector<vk::DescriptorSetLayout> setLayout;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::ShaderModule> shaderModules;

		~PipelineData();

		std::vector<vk::DescriptorSet> allocDescriptorSets();
	};
	template<typename ...Args>
	struct LoadPipelineSimple {
		using RetTy = std::shared_ptr<PipelineData>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&,const std::string&);
		static std::string key_from_args(vk::Device,vk::RenderPass,const vk::Extent2D&,const std::string&,const std::string&,
			uint32_t maxPoolSize = 1,
			const std::unordered_set<uint32_t>& ins_set = {},
			const std::vector<uint32_t>& vertextInputBindingSplit = {},
			std::function<void(vk::GraphicsPipelineCreateInfo)> on = {});
		static RealRetTy load(vk::Device, vk::RenderPass, const vk::Extent2D&,std::string, std::string,
			uint32_t maxPoolSize = 1, 
			std::unordered_set<uint32_t> ins_set = {},
			std::vector<uint32_t> vertextInputBindingSplit = {},
			std::function<void(vk::GraphicsPipelineCreateInfo)> on = {});
	};

	using LoadPipelineSimpleTy = LoadPipelineSimple<vk::Device, vk::RenderPass, const vk::Extent2D&, std::string, std::string,
		uint32_t,
		std::unordered_set<uint32_t>,
		std::vector<uint32_t>,
		std::function<void(vk::GraphicsPipelineCreateInfo)>>;
}