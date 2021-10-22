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

	LoadSpirvWithMetaData::ArgsTy LoadSpirvWithMetaData::default_args() { return spv_target_env::SPV_ENV_VULKAN_1_2; }

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

	LoadSpirvWithMetaData::RealRetTy LoadSpirvWithMetaData::load(PathTy p, VKD_RES_MGR_KEY_TYPE k,spv_target_env env)
	{
		auto text = DefResMgr::instance()->load<ResType::glsl>(k.string());
		if(!text) return std::make_tuple(false,nullptr);
		auto elang = get_lang_by_suffix(k);
		if (!elang)
		{
			dbg::log << "Not support sharder type " << k << "\n";
			return std::make_tuple(false, nullptr);
		}
		glslang::TShader shader(*elang);
		auto code = text->data();
		int code_n = text->size();
		shader.setStringsWithLengths(&code,&code_n,1);
		TBuiltInResource buildin;
		shader.parse(&buildin,450,false,EShMessages::EShMsgDefault);
		
		std::vector<uint32_t> bin;
		glslang::GlslangToSpv(*shader.getIntermediate(),bin);
		
		spvtools::SpirvTools tools(env);
		tools.SetMessageConsumer([](spv_message_level_t level, const char* source,
			const spv_position_t& position, const char* message) {
			dbg::log << "Assemble Spirv Message \n" << 
				'[' << position.line << ':' << position.column << '] ' << spv_message_level_to_str(level) << ' ' << message << "\n";
		});
		
		if (!tools.Assemble(*text, &bin))
		{
			return std::make_tuple(false, nullptr);
		}
		spvtools::Context ctx(env);
		spv_text res;
		spv_diagnostic dia;
		if (spvBinaryToText(ctx.CContext(), bin.data(), bin.size(), 0, &res, &dia) != SPV_SUCCESS)
		{

		}

		

		return std::make_tuple(true,std::make_shared<std::vector<uint32_t>>(bin));
	}

}