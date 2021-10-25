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

	void push_descriptor(spirv_cross::ShaderResources& sr, spirv_cross::SmallVector<spirv_cross::Resource> spirv_cross::ShaderResources::* f, ShaderResources& res,
		vk::DescriptorType ty, spirv_cross::CompilerGLSL& glsl)
	{
		for (auto& r : sr.*f)
		{
			Descriptor d;
			d.binding = glsl.get_decoration(r.id, spv::Decoration::DecorationBinding);
			d.set = glsl.get_decoration(r.id, spv::Decoration::DecorationDescriptorSet);
			d.name = std::move(r.name);
			d.type = vk::DescriptorType::eUniformBuffer;
			res.descriptors.push_back(d);
		}
	}

	vk::Format get_format_by_type(spirv_cross::SPIRType& type);

	void push_stageIO(spirv_cross::ShaderResources& sr, spirv_cross::SmallVector<spirv_cross::Resource> spirv_cross::ShaderResources::* f, ShaderResources& res,
		std::vector<uint32_t> ShaderResources::* bindingStride,std::vector<StageIO> ShaderResources::* ios,
		spirv_cross::CompilerGLSL& glsl)
	{
		int32_t curr_binding = -1;
		uint32_t last_size = 0;
		for (auto& r : sr.*f)
		{
			StageIO input;
			auto binding = glsl.get_decoration(r.id, spv::Decoration::DecorationBinding);
			if (binding != curr_binding)
			{
				if (curr_binding > -1)
				{
					(res.*bindingStride).push_back(last_size);
				}
				last_size = 0;
				curr_binding = binding;
			}
			input.binding = binding;
			input.location = glsl.get_decoration(r.id, spv::Decoration::DecorationLocation);
			auto type = glsl.get_type(r.type_id);
			input.offset = last_size;

			last_size += type.width / 8 * type.vecsize;
			input.format = get_format_by_type(type);
			(res.*ios).push_back(input);

			if (!type.array.empty())
			{
				auto local = input.location + 1;
				uint32_t l = 1;
				for (uint32_t a : type.array)
					l *= a;
				for (int i = 0; i < l-1; ++i)
				{
					StageIO in(input);
					in.location = local;
					in.offset = last_size;
					last_size += type.width / 8 * type.vecsize;
					(res.*ios).push_back(in);
					local += 1;
				}
			}
		}
		(res.*bindingStride).push_back(last_size);
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

		auto shaderRes = glsl.get_shader_resources();

		push_descriptor(shaderRes, &spirv_cross::ShaderResources::uniform_buffers, res.shaderRes,vk::DescriptorType::eUniformBuffer,glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::sampled_images, res.shaderRes, vk::DescriptorType::eCombinedImageSampler, glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::separate_images, res.shaderRes, vk::DescriptorType::eSampledImage, glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::separate_samplers, res.shaderRes, vk::DescriptorType::eSampler, glsl);

		push_stageIO(shaderRes,&spirv_cross::ShaderResources::stage_inputs,res.shaderRes,&ShaderResources::inBindingStride,&ShaderResources::stageInput,glsl);

		uint32_t offset = 0;
		for (auto& r : shaderRes.push_constant_buffers)
		{
			PushConstant pc;
			pc.size = glsl.get_declared_struct_size(glsl.get_type(r.type_id));
			pc.offset = offset;
			res.shaderRes.pushConstant.push_back(pc);
			offset += pc.size;
		}

		res.stage = getStage(*elang).value();

		return std::make_tuple(true,std::make_shared<SpirvRes>(res));
	}

#define DefGetFormatCase(T,SZ,T2)					\
case spirv_cross::SPIRType::T:					\
	switch (type.vecsize)							\
	{													\
	case 1:return vk::Format::eR##SZ##T2;					\
	case 2:return vk::Format::eR##SZ##G##SZ##T2;						\
	case 3:return vk::Format::eR##SZ##G##SZ##B##SZ##T2;					\
	case 4:return vk::Format::eR##SZ##G##SZ##B##SZ##A##SZ##T2;			\
	}

	vk::Format get_format_by_type(spirv_cross::SPIRType& type)
	{
		switch (type.basetype)
		{
			DefGetFormatCase(Half, 16, Sfloat)
			DefGetFormatCase(Float,32,Sfloat)
			DefGetFormatCase(Double,64,Sfloat)
			DefGetFormatCase(Int, 32, Sint)
			DefGetFormatCase(Int64, 64, Sint)
			DefGetFormatCase(UInt, 32, Uint)
			DefGetFormatCase(UInt64,64,Uint)
			DefGetFormatCase(Short, 16, Sint)
			DefGetFormatCase(UShort,16, Uint)
			DefGetFormatCase(SByte, 8, Sint)
			DefGetFormatCase(UByte, 8, Uint)
		default:
			break;
		}
		return vk::Format::eUndefined;
	}
	#undef DefGetFormatCase
}