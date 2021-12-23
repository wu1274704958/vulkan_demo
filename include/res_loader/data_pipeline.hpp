#pragma once
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <unordered_set>
#include <optional>

namespace gld::vkd {

	struct DescriptorSets
	{
		DescriptorSets(std::vector<vk::DescriptorSet> sets, vk::DescriptorPool pool): descriptorSets(std::move(sets)),pool(pool)
		{}
		DescriptorSets() {}
		operator bool () const;

		operator const std::vector<vk::DescriptorSet>& () const;

		void cleanup(vk::Device d);

		vk::DescriptorPool pool;
		std::optional<std::vector<vk::DescriptorSet>> descriptorSets;
	};
	struct PipelineData
	{
		vk::Device device;
		std::vector<vk::DescriptorSetLayout> setLayout;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;
		std::vector<vk::DescriptorPool> descriptorPool;
		std::vector<vk::ShaderModule> shaderModules;
		vk::DescriptorPoolCreateInfo poolCreateInfo;
		std::vector<vk::DescriptorPoolSize> poolSize;

		~PipelineData();

		DescriptorSets allocDescriptorSets();
		bool createNewDescriptorPool();
	};
	template<typename ...Args>
	struct LoadPipelineSimple {
		using RetTy = std::shared_ptr<PipelineData>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&,const std::string&,vk::RenderPass);
		static std::string key_from_args(vk::Device,vk::RenderPass,const vk::Extent2D&,const std::string&,const std::string&,
			uint32_t maxPoolSize = 1,
			const std::unordered_set<uint32_t>& ins_set = {},
			const std::vector<uint32_t>& vertextInputBindingSplit = {},
			std::function<void(vk::GraphicsPipelineCreateInfo&)> on = {});
		static RealRetTy load(vk::Device, vk::RenderPass, const vk::Extent2D&,std::string, std::string,
			uint32_t maxPoolSize = 1, 
			std::unordered_set<uint32_t> ins_set = {},
			std::vector<uint32_t> vertextInputBindingSplit = {},
			std::function<void(vk::GraphicsPipelineCreateInfo&)> on = {});
	};

	using LoadPipelineSimpleTy = LoadPipelineSimple<vk::Device, vk::RenderPass, const vk::Extent2D&, std::string, std::string,
		uint32_t,
		std::unordered_set<uint32_t>,
		std::vector<uint32_t>,
		std::function<void(vk::GraphicsPipelineCreateInfo&)>>;
}