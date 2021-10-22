#include <res_loader/resource_mgr.hpp>
#include <spirv-tools/libspirv.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/ShaderLang.h>
#include <sundry.hpp>
#include <log.hpp>
#include <comm.hpp>
#include <res_loader/res_shader.hpp>

namespace gld::vkd {

	LoadSpirvWithMetaData::ArgsTy LoadSpirvWithMetaData::default_args() { return glslang::EShTargetClientVersion::EShTargetVulkan_1_2; }

	std::string LoadSpirvWithMetaData::format_args(ArgsTy flag) {
		auto t = std::make_tuple((int)flag);
		return sundry::format_tup(t,'#');
	}

	const char* spv_message_level_to_str(spv_message_level_t l)
	{
		switch(l)
		{
		case SPV_MSG_FATAL:				return "SPV_MSG_FATAL";
		case SPV_MSG_INTERNAL_ERROR:	return "SPV_MSG_INTERNAL_ERROR";
		case SPV_MSG_ERROR:				return "SPV_MSG_ERROR";
		case SPV_MSG_WARNING:			return "SPV_MSG_WARNING";
		case SPV_MSG_INFO:				return "SPV_MSG_INFO";
		case SPV_MSG_DEBUG:				return "SPV_MSG_DEBUG";
		}
		return "";
	}

	std::optional<EShLanguage> get_lang_by_suffix(VKD_RES_MGR_KEY_TYPE k)
	{
		auto e = k.extension();
		if (e == "vert")
		{
			return EShLanguage::EShLangVertex;
		}
		int idx = eq_ct_str_ret(e.c_str(),PREPARE_CT_STR(L".vert"), PREPARE_CT_STR(L".frag"), PREPARE_CT_STR(L".comp"), PREPARE_CT_STR(L".geom"),
			PREPARE_CT_STR(L".tese"), PREPARE_CT_STR(L".tesc"));
		constexpr std::array<EShLanguage,6> flags = std::array<EShLanguage,6>{ EShLanguage::EShLangVertex, EShLanguage::EShLangFragment, EShLanguage::EShLangCompute,EShLanguage::EShLangGeometry,
			EShLanguage::EShLangTessEvaluation, EShLanguage::EShLangTessControl};
		if(idx == -1)
			return std::nullopt;
		else {
			return flags[idx];
		}
	}

	LoadSpirvWithMetaData::RealRetTy LoadSpirvWithMetaData::load(PathTy p, VKD_RES_MGR_KEY_TYPE k, glslang::EShTargetClientVersion env)
	{
		auto text = DefResMgr::instance()->load<ResType::glsl>(k.string());
		if(!text) return std::make_tuple(false,nullptr);
		auto elang = get_lang_by_suffix(k);
		if (!elang)
		{
			dbg::log << "Not support sharder type " << k << "\n";
			return std::make_tuple(false, nullptr);
		}
		glslang::InitializeProcess();
		glslang::TShader shader(*elang);
		auto code = text->data();
		int code_n = text->size();
		shader.setStringsWithLengths(&code,&code_n,1);
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl,*elang,glslang::EShClient::EShClientVulkan,100);
		shader.setEnvClient(glslang::EShClient::EShClientVulkan,env);
		shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv,glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);
		TBuiltInResource buildin;
		auto result = shader.parse(&buildin, 100, false, EShMessages::EShMsgDefault);
		
		std::vector<uint32_t> bin;
		glslang::GlslangToSpv(*shader.getIntermediate(),bin);
		
		glslang::FinalizeProcess();

		return std::make_tuple(true,std::make_shared<std::vector<uint32_t>>(bin));
	}

}