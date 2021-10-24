#include <res_loader/resource_mgr.hpp>
#include <spirv-tools/libspirv.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <sundry.hpp>
#include <log.hpp>
#include <comm.hpp>
#include <res_loader/res_shader.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <common.hpp>

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

	std::optional<spv_target_env> get_spv_target_env(glslang::EShTargetClientVersion env)
	{
		switch (env)
		{
			case glslang::EShTargetVulkan_1_0: return spv_target_env::SPV_ENV_VULKAN_1_0;
			case glslang::EShTargetVulkan_1_1: return spv_target_env::SPV_ENV_VULKAN_1_1;
			case glslang::EShTargetVulkan_1_2: return spv_target_env::SPV_ENV_VULKAN_1_2;
		default:
			break;
		}
		return std::nullopt;
	}

	std::optional<vk::ShaderStageFlagBits> getStage(EShLanguage l)
	{
		return wws::map_enum<wws::ValList<EShLanguage,
			EShLanguage::EShLangVertex,
			EShLanguage::EShLangFragment,
			EShLanguage::EShLangCompute,
			EShLanguage::EShLangGeometry,
			EShLanguage::EShLangTessEvaluation,
			EShLanguage::EShLangTessControl>,
			wws::ValList<vk::ShaderStageFlagBits,
			vk::ShaderStageFlagBits::eVertex,
			vk::ShaderStageFlagBits::eFragment,
			vk::ShaderStageFlagBits::eCompute,
			vk::ShaderStageFlagBits::eGeometry,
			vk::ShaderStageFlagBits::eTessellationEvaluation,
			vk::ShaderStageFlagBits::eTessellationControl>>(l);
	}

	LoadSpirvWithMetaData::RealRetTy LoadSpirvWithMetaData::load(PathTy p, VKD_RES_MGR_KEY_TYPE k, glslang::EShTargetClientVersion env)
	{
		auto tar_env = get_spv_target_env(env);
		if(!tar_env)
			return std::make_tuple(false,nullptr);
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
		buildin.maxDrawBuffers = true;
		
		if (!shader.parse(&buildin, 100, false, EShMessages::EShMsgDefault))
		{
			auto info = shader.getInfoLog();
			dbg::log << "parse glsl failed " << k << " " << info << dbg::endl;
			glslang::FinalizeProcess();
			return std::make_tuple(false,nullptr);
		}
		SpirvRes res;
		res.entryPoint = shader.getIntermediate()->getEntryPointName();
		glslang::GlslangToSpv(*shader.getIntermediate(),res.binary);
		
		glslang::FinalizeProcess();

		spirv_cross::CompilerGLSL glsl(res.binary.data(),res.binary.size());

		res.shaderRes = glsl.get_shader_resources();

		for (auto r : res.shaderRes.uniform_buffers)
		{
			auto set = glsl.get_decoration(r.id,spv::Decoration::DecorationDescriptorSet);
			auto binding = glsl.get_decoration(r.id,spv::Decoration::DecorationBinding);
			printf(" %s at set = %u, binding = %u\n", r.name.c_str(), set, binding);
		}

		for (auto r : res.shaderRes.stage_inputs)
		{
			auto loca = glsl.get_decoration(r.id, spv::Decoration::DecorationLocation);
			auto type = glsl.get_type(r.type_id);

			printf(" %s at set = %u, binding = %u\n", r.name.c_str(), loca, 0);
		}

		res.stage = getStage(*elang).value();

		return std::make_tuple(true,std::make_shared<SpirvRes>(res));
	}

}