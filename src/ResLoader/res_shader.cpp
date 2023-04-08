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

	extern const TBuiltInResource k_default_conf;

	template <>
	std::string LoadSpirvWithMetaDataTy::key_from_args(glslang::EShTargetClientVersion v, const std::vector<uint32_t>& binding)
	{
		return sundry::format_tup('#',(int)v, binding);
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

	std::optional<EShLanguage> get_lang_by_suffix(const std::string& k)
	{
		int i = k.find_last_of('.');
		if (i <= -1) return std::nullopt;
		std::string e = k.substr(i);
		int idx = eq_ct_str_ret(e.c_str(),PREPARE_CT_STR(".vert"), PREPARE_CT_STR(".frag"), PREPARE_CT_STR(".comp"), PREPARE_CT_STR(".geom"),
			PREPARE_CT_STR(".tese"), PREPARE_CT_STR(".tesc"));
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

	std::optional<vk::DescriptorType> getType(spirv_cross::SPIRType::BaseType l)
	{
		return wws::map_enum<wws::ValList<spirv_cross::SPIRType::BaseType,
			spirv_cross::SPIRType::BaseType::SampledImage,
			spirv_cross::SPIRType::BaseType::Sampler,
			spirv_cross::SPIRType::BaseType::Image>,
			wws::ValList<vk::DescriptorType,
			vk::DescriptorType::eCombinedImageSampler,
			vk::DescriptorType::eSampler,
			vk::DescriptorType::eStorageImage>>(l);
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
			d.type = ty;
			res.descriptors.push_back(d);
		}
	}

	vk::Format get_format_by_type(spirv_cross::SPIRType& type);

	struct LocationCmp
	{
		LocationCmp(spirv_cross::CompilerGLSL& glsl) : glsl(glsl){}
		bool cmp(const spirv_cross::Resource& a, const spirv_cross::Resource& b) const 
		{
			auto al = glsl.get_decoration(a.id, spv::Decoration::DecorationLocation);
			auto bl = glsl.get_decoration(b.id, spv::Decoration::DecorationLocation);
			return al < bl;
		}
		spirv_cross::CompilerGLSL& glsl;
	};

	void push_stageIO(spirv_cross::ShaderResources& sr, spirv_cross::SmallVector<spirv_cross::Resource> spirv_cross::ShaderResources::* f, ShaderResources& res,
		std::vector<BindingStride> ShaderResources::* bindingStride,std::vector<StageIO> ShaderResources::* ios,
		spirv_cross::CompilerGLSL& glsl,const std::vector<uint32_t>& binding_split)
	{
		int32_t curr_binding = -1;
		uint32_t last_size = 0;
		uint32_t binding_ck = 0;

		LocationCmp cmp(glsl);
		auto& res_vec = sr.*f;

		wws::sort_heap<spirv_cross::Resource>(res_vec,cmp);

		for (auto& r : sr.*f)
		{
			auto start_locat = glsl.get_decoration(r.id, spv::Decoration::DecorationLocation);
			auto binding = glsl.get_decoration(r.id, spv::Decoration::DecorationBinding);
			if(binding_ck < binding_split.size() && start_locat >= binding_split[binding_ck])
				++binding_ck;
			if(binding < binding_ck)
				binding = binding_ck;
			
			if (binding != curr_binding)
			{
				if (curr_binding > -1)
				{
					(res.*bindingStride).push_back({(uint32_t)curr_binding,last_size});
				}
				last_size = 0;
				curr_binding = binding;
			}
			auto type = glsl.get_type(r.type_id);
			
			auto num = type.columns;
			if (!type.array.empty())
			{
				for (uint32_t a : type.array)
					num *= a;
			}
			auto format = get_format_by_type(type);
			for (int i = 0; i < num; ++i)
			{
				StageIO in;
				in.binding = binding;
				in.location = start_locat;
				in.offset = last_size;
				in.format = format;
				last_size += type.width / 8 * type.vecsize;
				(res.*ios).push_back(in);
				start_locat += 1;
			}
		}
		(res.*bindingStride).push_back({ (uint32_t)curr_binding,last_size });
	}

	class Includer : public glslang::TShader::Includer
	{
		
		virtual IncludeResult* includeSystem(const char* headerName,
			const char* includerName,
			size_t inclusionDepth) override {
			return include_dir(system_dir, headerName);
		} 

		// For the "local"-only aspect of a "" include. Should not search in the
		// "system" paths, because on returning a failure, the parser will
		// call includeSystem() to look in the "system" locations.
		virtual IncludeResult* includeLocal(const char* headerName,
			const char* includerName,
			size_t inclusionDepth) override{
			return include_dir(cur,headerName);
		}

		IncludeResult* include_dir(const std::string & dir,const char* headerName)
		{
			auto res = wws::append_path(dir,headerName);
			if(!res) return nullptr;
			auto v = gld::DefResMgr::instance()->load<ResType::text>(*res);
			if(!v) return nullptr;
			IncludeResult*  r = new IncludeResult(headerName,v->data(),v->size(),nullptr);
			return r;
		}

		// Signals that the parser will no longer use the contents of the
		// specified IncludeResult.
		virtual void releaseInclude(IncludeResult* ptr) {
			delete ptr;
		};
	public:
		Includer() :cur(""),system_dir("") {}
		Includer(std::string&& path) : cur(path) {
			
		}
		template<typename T>
		void set_sys_dir(T&& t)
			requires requires(std::string s,T&& t)
		{
			s = std::forward<T>(t);
		}
		{
			system_dir = std::forward<T>(t);
		}
	protected:
		std::string cur;
		std::string system_dir;
	};

	std::string preprocess(const std::string& code, const std::string& path, EShLanguage elang, glslang::EShTargetClientVersion env)
	{
		glslang::TShader shader(elang);
		const int code_n = code.size();
		auto c = code.data();
		shader.setStringsWithLengths(&c, &code_n, 1);
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl, elang, glslang::EShClient::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClient::EShClientVulkan, env);
		shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);
		std::string res;
		auto key = gld::DefResMgr::instance()->path_to_key(path);
		if (!key) return code;
		wws::up_path(*key);
		Includer incl(std::move(*key));
		incl.set_sys_dir("shaders/comm");
		if (!shader.preprocess(&k_default_conf, 100, ENoProfile, false, false, EShMessages::EShMsgDefault, &res, incl))
		{
			auto info = shader.getInfoLog();
			dbg::log << "preprocess glsl failed " << path << " " << info << dbg::endl;
		}
		return res;
	}

	template <>
	LoadSpirvWithMetaDataTy::RealRetTy
		LoadSpirvWithMetaDataTy::load(FStream* stream, const std::string& path, glslang::EShTargetClientVersion env, std::vector<uint32_t> binding)
	{
		auto tar_env = get_spv_target_env(env);
		if (!tar_env)
			return std::make_tuple(false, nullptr);
		auto text = DefResMgr::instance()->load<ResType::text>(stream, path);
		if (!text) return std::make_tuple(false, nullptr);
		auto elang = get_lang_by_suffix(path);
		if (!elang)
		{
			dbg::log << "Not support sharder type " << path << "\n";
			return std::make_tuple(false, nullptr);
		}
		glslang::InitializeProcess();
		glslang::TShader shader(*elang);
		glslang::TProgram prog;

		auto preCode = preprocess(*text, path, *elang, env);

		auto code = preCode.data();
		int code_n = preCode.size();

		shader.setStringsWithLengths(&code, &code_n, 1);
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl, *elang, glslang::EShClient::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClient::EShClientVulkan, env);
		shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);

		if (!shader.parse(&k_default_conf, 100, false, EShMessages::EShMsgDefault))
		{
			auto info = shader.getInfoLog();
			dbg::log << "parse glsl failed " << path << " " << info << dbg::endl;
			glslang::FinalizeProcess();
			return std::make_tuple(false, nullptr);
		}

		prog.addShader(&shader);

		if (!prog.link(EShMsgDefault))
		{
			auto info = prog.getInfoLog();
			dbg::log << "link program failed " << path << " " << info << dbg::endl;
			glslang::FinalizeProcess();
			return std::make_tuple(false, nullptr);
		}

		SpirvRes res;
		res.entryPoint = shader.getIntermediate()->getEntryPointName();
		glslang::GlslangToSpv(*prog.getIntermediate(*elang), res.binary);

		glslang::FinalizeProcess();

		spvtools::SpirvTools spv_tools(*tar_env);

		spv_tools.SetMessageConsumer([&path](spv_message_level_t  level, const char* source,
			const spv_position_t& position, const char* message) {
			dbg::log << spv_message_level_to_str(level) << " [" << position.line << ':' << position.column << "] " << path << " " << source << " " << message << dbg::endl;
		});

		auto v = spv_tools.Validate(res.binary);

		spirv_cross::CompilerGLSL glsl(res.binary.data(), res.binary.size());

		auto shaderRes = glsl.get_shader_resources();

		push_descriptor(shaderRes, &spirv_cross::ShaderResources::uniform_buffers, res.shaderRes, vk::DescriptorType::eUniformBuffer, glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::sampled_images, res.shaderRes, vk::DescriptorType::eCombinedImageSampler, glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::separate_images, res.shaderRes, vk::DescriptorType::eSampledImage, glsl);
		push_descriptor(shaderRes, &spirv_cross::ShaderResources::separate_samplers, res.shaderRes, vk::DescriptorType::eSampler, glsl);

		push_stageIO(shaderRes, &spirv_cross::ShaderResources::stage_inputs, res.shaderRes, 
			&ShaderResources::inBindingStride, &ShaderResources::stageInput, glsl, binding);

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

		return std::make_tuple(true, std::make_shared<SpirvRes>(res));
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
	TBuiltInResource init_k_default_conf()
	{
		TBuiltInResource res = {};
		res.maxLights =  32,
		res.maxClipPlanes =  6,
		res.maxTextureUnits =  32,
		res.maxTextureCoords =  32,
		res.maxVertexAttribs =  64,
		res.maxVertexUniformComponents =  4096,
		res.maxVaryingFloats =  64,
		res.maxVertexTextureImageUnits =  32,
		res.maxCombinedTextureImageUnits =  80,
		res.maxTextureImageUnits =  32,
		res.maxFragmentUniformComponents =  4096,
		res.maxDrawBuffers =  32,
		res.maxVertexUniformVectors =  128,
		res.maxVaryingVectors =  8,
		res.maxFragmentUniformVectors =  16,
		res.maxVertexOutputVectors =  16,
		res.maxFragmentInputVectors =  15,
		res.minProgramTexelOffset =  -8,
		res.maxProgramTexelOffset =  7,
		res.maxClipDistances =  8,
		res.maxComputeWorkGroupCountX =  65535,
		res.maxComputeWorkGroupCountY =  65535,
		res.maxComputeWorkGroupCountZ =  65535,
		res.maxComputeWorkGroupSizeX =  1024,
		res.maxComputeWorkGroupSizeY =  1024,
		res.maxComputeWorkGroupSizeZ =  64,
		res.maxComputeUniformComponents =  1024,
		res.maxComputeTextureImageUnits =  16,
		res.maxComputeImageUniforms =  8,
		res.maxComputeAtomicCounters =  8,
		res.maxComputeAtomicCounterBuffers =  1,
		res.maxVaryingComponents =  60,
		res.maxVertexOutputComponents =  64,
		res.maxGeometryInputComponents =  64,
		res.maxGeometryOutputComponents =  128,
		res.maxFragmentInputComponents =  128,
		res.maxImageUnits =  8,
		res.maxCombinedImageUnitsAndFragmentOutputs =  8,
		res.maxCombinedShaderOutputResources =  8,
		res.maxImageSamples =  0,
		res.maxVertexImageUniforms =  0,
		res.maxTessControlImageUniforms =  0,
		res.maxTessEvaluationImageUniforms =  0,
		res.maxGeometryImageUniforms =  0,
		res.maxFragmentImageUniforms =  8,
		res.maxCombinedImageUniforms =  8,
		res.maxGeometryTextureImageUnits =  16,
		res.maxGeometryOutputVertices =  256,
		res.maxGeometryTotalOutputComponents =  1024,
		res.maxGeometryUniformComponents =  1024,
		res.maxGeometryVaryingComponents =  64,
		res.maxTessControlInputComponents =  128,
		res.maxTessControlOutputComponents =  128,
		res.maxTessControlTextureImageUnits =  16,
		res.maxTessControlUniformComponents =  1024,
		res.maxTessControlTotalOutputComponents =  4096,
		res.maxTessEvaluationInputComponents =  128,
		res.maxTessEvaluationOutputComponents =  128,
		res.maxTessEvaluationTextureImageUnits =  16,
		res.maxTessEvaluationUniformComponents =  1024,
		res.maxTessPatchComponents =  120,
		res.maxPatchVertices =  32,
		res.maxTessGenLevel =  64,
		res.maxViewports =  16,
		res.maxVertexAtomicCounters =  0,
		res.maxTessControlAtomicCounters =  0,
		res.maxTessEvaluationAtomicCounters =  0,
		res.maxGeometryAtomicCounters =  0,
		res.maxFragmentAtomicCounters =  8,
		res.maxCombinedAtomicCounters =  8,
		res.maxAtomicCounterBindings =  1,
		res.maxVertexAtomicCounterBuffers =  0,
		res.maxTessControlAtomicCounterBuffers =  0,
		res.maxTessEvaluationAtomicCounterBuffers =  0,
		res.maxGeometryAtomicCounterBuffers =  0,
		res.maxFragmentAtomicCounterBuffers =  1,
		res.maxCombinedAtomicCounterBuffers =  1,
		res.maxAtomicCounterBufferSize =  16384,
		res.maxTransformFeedbackBuffers =  4,
		res.maxTransformFeedbackInterleavedComponents =  64,
		res.maxCullDistances =  8,
		res.maxCombinedClipAndCullDistances =  8,
		res.maxSamples =  4,
		res.maxMeshOutputVerticesNV =  256,
		res.maxMeshOutputPrimitivesNV =  512,
		res.maxMeshWorkGroupSizeX_NV =  32,
		res.maxMeshWorkGroupSizeY_NV =  1,
		res.maxMeshWorkGroupSizeZ_NV =  1,
		res.maxTaskWorkGroupSizeX_NV =  32,
		res.maxTaskWorkGroupSizeY_NV =  1,
		res.maxTaskWorkGroupSizeZ_NV =  1,
		res.maxMeshViewCountNV =  4,
		res.maxDualSourceDrawBuffersEXT = 1,
		res.limits = {
			/* .nonInductiveForLoops =		*/true,
			/* .whileLoops =				*/true,
			/* .doWhileLoops =				*/true,
			/* .generalUniformIndexing =	*/true,
			/* .generalAttributeMatrixVectorIndexing =  */true,
			/* .generalVaryingIndexing =	*/true,
			/* .generalSamplerIndexing =	*/true,
			/* .generalVariableIndexing =	*/true,
			/* .generalConstantMatrixVectorIndexing =  */true,
		};
		return res;
	}
	const TBuiltInResource k_default_conf = init_k_default_conf();
}

