#pragma once
#include <res_loader/res_comm.hpp>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <vulkan/vulkan.hpp>
namespace gld::vkd {

	struct Descriptor
	{
		uint32_t set;
		uint32_t binding;
		std::string name;
		vk::DescriptorType type;
	};
	struct StageIO
	{
		uint32_t	binding;
		uint32_t	location;
		vk::Format	format;
		uint32_t	offset;
	};
	struct PushConstant
	{
		uint32_t size;
		uint32_t offset;
	};
	struct BindingStride
	{
		uint32_t binding;
		uint32_t stride;
	};

	struct ShaderResources
	{
		std::vector<Descriptor> descriptors;
		std::vector<BindingStride> inBindingStride;
		std::vector<StageIO> stageInput;
		std::vector<PushConstant> pushConstant;
	};
	
	struct SpirvRes {
		std::vector<uint32_t> binary;
		ShaderResources shaderRes;
		vk::ShaderStageFlagBits stage;
		std::string entryPoint;
	};

	struct LoadSpirvWithMetaData
	{
		using RetTy = std::shared_ptr<SpirvRes>;
		using ArgsTy = glslang::EShTargetClientVersion;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string format_args(ArgsTy flag);
		static ArgsTy default_args();
		//static RealRetTy load(PathTy, glslang::EShTargetClientVersion);
	};
}