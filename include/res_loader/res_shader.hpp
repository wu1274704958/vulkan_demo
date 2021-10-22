#pragma once
#include <res_loader/res_comm.hpp>
#include <glslang/Public/ShaderLang.h>
namespace gld::vkd {
	struct LoadSpirvWithMetaData
	{
		using RetTy = std::shared_ptr<std::vector<uint32_t>>;
		using ArgsTy = glslang::EShTargetClientVersion;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string format_args(ArgsTy flag);
		static ArgsTy default_args();
		static RealRetTy load(VKD_RES_MGR_CXT_PTR_TYPE_WITH_COMMA PathTy, VKD_RES_MGR_KEY_TYPE_WITH_COMMA glslang::EShTargetClientVersion);
	};
}