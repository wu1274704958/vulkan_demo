#pragma once
#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

namespace gld::vkd {

	struct PipelineData
	{
		vk::Device device;

	};

	struct LoadPipelineSimple {
		using RetTy = std::shared_ptr<PipelineData>;
		using ArgsTy = std::tuple<vk::Device,std::string,std::string>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const ArgsTy& args);
		static RealRetTy load(ArgsTy args);
	};
}