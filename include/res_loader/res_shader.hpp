#pragma once
#include <res_loader/res_comm.hpp>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <vulkan/vulkan.hpp>
namespace gld::vkd {

	struct SpirvRes {
		std::vector<uint32_t> binary;
		spirv_cross::ShaderResources shaderRes;
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
		static RealRetTy load(VKD_RES_MGR_CXT_PTR_TYPE_WITH_COMMA PathTy, VKD_RES_MGR_KEY_TYPE_WITH_COMMA glslang::EShTargetClientVersion);
	};
}