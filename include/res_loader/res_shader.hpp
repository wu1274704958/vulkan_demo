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
	template<typename ... Args>
	struct LoadSpirvWithMetaData
	{
		using RetTy = std::shared_ptr<SpirvRes>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(glslang::EShTargetClientVersion v = glslang::EShTargetClientVersion::EShTargetVulkan_1_2,const std::vector<uint32_t>& binding = {});
		static RealRetTy load(FStream*,const std::string&, glslang::EShTargetClientVersion v = glslang::EShTargetClientVersion::EShTargetVulkan_1_2,std::vector<uint32_t> binding = {});
	};

	using LoadSpirvWithMetaDataTy = LoadSpirvWithMetaData<glslang::EShTargetClientVersion, std::vector<uint32_t>>;
}