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

	std::string LoadSpirvWithMetaData<glslang::EShTargetClientVersion>::key_from_args(glslang::EShTargetClientVersion v) {
		return wws::to_string((int)v);
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

	void push_stageIO(spirv_cross::ShaderResources& sr, spirv_cross::SmallVector<spirv_cross::Resource> spirv_cross::ShaderResources::* f, ShaderResources& res,
		std::vector<BindingStride> ShaderResources::* bindingStride,std::vector<StageIO> ShaderResources::* ios,
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
					(res.*bindingStride).push_back({(uint32_t)curr_binding,last_size});
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
			IncludeResult*  r = new IncludeResult("",v->data(),v->size(),nullptr);
			return r;
		}

		// Signals that the parser will no longer use the contents of the
		// specified IncludeResult.
		virtual void releaseInclude(IncludeResult* ptr) {
			
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
		std::string r = path;
		wws::up_path(r);
		Includer incl(std::move(r));
		incl.set_sys_dir("shaders/comm");
		if (!shader.preprocess(&k_default_conf, 100, ENoProfile, false, false, EShMessages::EShMsgDefault, &res, incl))
		{
			auto info = shader.getInfoLog();
			dbg::log << "preprocess glsl failed " << path << " " << info << dbg::endl;
		}
		return res;
	}

	LoadSpirvWithMetaData<glslang::EShTargetClientVersion>::RealRetTy 
		LoadSpirvWithMetaData<glslang::EShTargetClientVersion>::load(FStream* stream,const std::string& path, glslang::EShTargetClientVersion env)
	{
		auto tar_env = get_spv_target_env(env);
		if(!tar_env)
			return std::make_tuple(false,nullptr);
		auto text = DefResMgr::instance()->load<ResType::text>(stream,path);
		if(!text) return std::make_tuple(false,nullptr);
		auto elang = get_lang_by_suffix(path);
		if (!elang)
		{
			dbg::log << "Not support sharder type " << path << "\n";
			return std::make_tuple(false, nullptr);
		}
		glslang::InitializeProcess();
		glslang::TShader shader(*elang);
		glslang::TProgram prog;

		auto preCode = preprocess(*text,path,*elang,env);

		auto code = preCode.data();
		int code_n = preCode.size();
		
		shader.setStringsWithLengths(&code,&code_n,1);
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl,*elang,glslang::EShClient::EShClientVulkan,100);
		shader.setEnvClient(glslang::EShClient::EShClientVulkan,env);
		shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv,glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);

		if (!shader.parse(&k_default_conf, 100, false, EShMessages::EShMsgDefault))
		{
			auto info = shader.getInfoLog();
			dbg::log << "parse glsl failed " << path << " " << info << dbg::endl;
			glslang::FinalizeProcess();
			return std::make_tuple(false,nullptr);
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
		glslang::GlslangToSpv(*prog.getIntermediate(*elang),res.binary);
		
		glslang::FinalizeProcess();

		spvtools::SpirvTools spv_tools(*tar_env);

		spv_tools.SetMessageConsumer([&path](spv_message_level_t  level , const char* source,
			const spv_position_t&  position , const char*  message ) {
				dbg::log << spv_message_level_to_str(level) << " [" << position.line <<':'<< position.column << "] " << path << " " << source << " " << message << dbg::endl;
			});

		auto v = spv_tools.Validate(res.binary);

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

	const TBuiltInResource k_default_conf = {
		/* .MaxLights = */ 32,
		/* .MaxClipPlanes = */ 6,
		/* .MaxTextureUnits = */ 32,
		/* .MaxTextureCoords = */ 32,
		/* .MaxVertexAttribs = */ 64,
		/* .MaxVertexUniformComponents = */ 4096,
		/* .MaxVaryingFloats = */ 64,
		/* .MaxVertexTextureImageUnits = */ 32,
		/* .MaxCombinedTextureImageUnits = */ 80,
		/* .MaxTextureImageUnits = */ 32,
		/* .MaxFragmentUniformComponents = */ 4096,
		/* .MaxDrawBuffers = */ 32,
		/* .MaxVertexUniformVectors = */ 128,
		/* .MaxVaryingVectors = */ 8,
		/* .MaxFragmentUniformVectors = */ 16,
		/* .MaxVertexOutputVectors = */ 16,
		/* .MaxFragmentInputVectors = */ 15,
		/* .MinProgramTexelOffset = */ -8,
		/* .MaxProgramTexelOffset = */ 7,
		/* .MaxClipDistances = */ 8,
		/* .MaxComputeWorkGroupCountX = */ 65535,
		/* .MaxComputeWorkGroupCountY = */ 65535,
		/* .MaxComputeWorkGroupCountZ = */ 65535,
		/* .MaxComputeWorkGroupSizeX = */ 1024,
		/* .MaxComputeWorkGroupSizeY = */ 1024,
		/* .MaxComputeWorkGroupSizeZ = */ 64,
		/* .MaxComputeUniformComponents = */ 1024,
		/* .MaxComputeTextureImageUnits = */ 16,
		/* .MaxComputeImageUniforms = */ 8,
		/* .MaxComputeAtomicCounters = */ 8,
		/* .MaxComputeAtomicCounterBuffers = */ 1,
		/* .MaxVaryingComponents = */ 60,
		/* .MaxVertexOutputComponents = */ 64,
		/* .MaxGeometryInputComponents = */ 64,
		/* .MaxGeometryOutputComponents = */ 128,
		/* .MaxFragmentInputComponents = */ 128,
		/* .MaxImageUnits = */ 8,
		/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
		/* .MaxCombinedShaderOutputResources = */ 8,
		/* .MaxImageSamples = */ 0,
		/* .MaxVertexImageUniforms = */ 0,
		/* .MaxTessControlImageUniforms = */ 0,
		/* .MaxTessEvaluationImageUniforms = */ 0,
		/* .MaxGeometryImageUniforms = */ 0,
		/* .MaxFragmentImageUniforms = */ 8,
		/* .MaxCombinedImageUniforms = */ 8,
		/* .MaxGeometryTextureImageUnits = */ 16,
		/* .MaxGeometryOutputVertices = */ 256,
		/* .MaxGeometryTotalOutputComponents = */ 1024,
		/* .MaxGeometryUniformComponents = */ 1024,
		/* .MaxGeometryVaryingComponents = */ 64,
		/* .MaxTessControlInputComponents = */ 128,
		/* .MaxTessControlOutputComponents = */ 128,
		/* .MaxTessControlTextureImageUnits = */ 16,
		/* .MaxTessControlUniformComponents = */ 1024,
		/* .MaxTessControlTotalOutputComponents = */ 4096,
		/* .MaxTessEvaluationInputComponents = */ 128,
		/* .MaxTessEvaluationOutputComponents = */ 128,
		/* .MaxTessEvaluationTextureImageUnits = */ 16,
		/* .MaxTessEvaluationUniformComponents = */ 1024,
		/* .MaxTessPatchComponents = */ 120,
		/* .MaxPatchVertices = */ 32,
		/* .MaxTessGenLevel = */ 64,
		/* .MaxViewports = */ 16,
		/* .MaxVertexAtomicCounters = */ 0,
		/* .MaxTessControlAtomicCounters = */ 0,
		/* .MaxTessEvaluationAtomicCounters = */ 0,
		/* .MaxGeometryAtomicCounters = */ 0,
		/* .MaxFragmentAtomicCounters = */ 8,
		/* .MaxCombinedAtomicCounters = */ 8,
		/* .MaxAtomicCounterBindings = */ 1,
		/* .MaxVertexAtomicCounterBuffers = */ 0,
		/* .MaxTessControlAtomicCounterBuffers = */ 0,
		/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
		/* .MaxGeometryAtomicCounterBuffers = */ 0,
		/* .MaxFragmentAtomicCounterBuffers = */ 1,
		/* .MaxCombinedAtomicCounterBuffers = */ 1,
		/* .MaxAtomicCounterBufferSize = */ 16384,
		/* .MaxTransformFeedbackBuffers = */ 4,
		/* .MaxTransformFeedbackInterleavedComponents = */ 64,
		/* .MaxCullDistances = */ 8,
		/* .MaxCombinedClipAndCullDistances = */ 8,
		/* .MaxSamples = */ 4,
		/* .maxMeshOutputVerticesNV = */ 256,
		/* .maxMeshOutputPrimitivesNV = */ 512,
		/* .maxMeshWorkGroupSizeX_NV = */ 32,
		/* .maxMeshWorkGroupSizeY_NV = */ 1,
		/* .maxMeshWorkGroupSizeZ_NV = */ 1,
		/* .maxTaskWorkGroupSizeX_NV = */ 32,
		/* .maxTaskWorkGroupSizeY_NV = */ 1,
		/* .maxTaskWorkGroupSizeZ_NV = */ 1,
		/* .maxMeshViewCountNV = */ 4,
		/* .maxDualSourceDrawBuffersEXT = */1,
		/* .limits = */ {
			/* .nonInductiveForLoops = */ true,
			/* .whileLoops = */ true,
			/* .doWhileLoops = */ true,
			/* .generalUniformIndexing = */ true,
			/* .generalAttributeMatrixVectorIndexing = */ true,
			/* .generalVaryingIndexing = */ true,
			/* .generalSamplerIndexing = */ true,
			/* .generalVariableIndexing = */ true,
			/* .generalConstantMatrixVectorIndexing = */ true,
		} };
}

