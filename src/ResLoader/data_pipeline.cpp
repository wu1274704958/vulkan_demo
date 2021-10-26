#include <data_pipeline.hpp>
#include <sundry.hpp>
#include <res_loader/resource_mgr.hpp>
#include <common.hpp>
namespace gld::vkd {

	std::string LoadPipelineSimple::key_from_args(const ArgsTy& args)
	{
		std::string s = std::get<1>(args);
		s += '#';
		s += std::get<2>(args);
		for (auto i : std::get<3>(args))
		{
			s += wws::to_string(i);
		}
		return s;
	}
	inline void push_descriptor(std::vector<vk::DescriptorSetLayoutBinding>& bindings,std::vector<Descriptor>& descriptors,vk::ShaderStageFlagBits stage)
	{
		for (auto& d : descriptors)
		{
			vk::DescriptorSetLayoutBinding binding;
			binding.binding = d.binding;
			binding.descriptorCount = 1;
			binding.descriptorType = d.type;
			binding.stageFlags = stage;
			bindings.push_back(binding);
		}
	}
	inline void push_constant(std::vector<vk::PushConstantRange>& ranges, std::vector<PushConstant>& v, vk::ShaderStageFlagBits stage)
	{
		for (auto& r : v)
		{
			vk::PushConstantRange range;
			range.offset = r.offset;
			range.size = r.size;
			range.stageFlags = stage;
			ranges.push_back(range);
		}
	}
	
	inline void fillPipelineInputState(ShaderResources& res, vk::PipelineVertexInputStateCreateInfo& inputState,const std::unordered_set<uint32_t>& is_instance_set)
	{
		std::vector<vk::VertexInputBindingDescription> binding;
		for (auto& r : res.inBindingStride)
		{
			binding.push_back(vk::VertexInputBindingDescription(r.binding,r.stride,
				is_instance_set.contains(r.binding) ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex));
		}
		std::vector<vk::VertexInputAttributeDescription> attr;
		for (auto& r : res.stageInput)
		{
			attr.push_back(vk::VertexInputAttributeDescription(r.location,r.binding,r.format,r.offset));
		}
		
		inputState.setVertexBindingDescriptions(binding);
		inputState.setVertexAttributeDescriptions(attr);
	}
	template<size_t N>
	LoadPipelineSimple::RealRetTy realCreatePipeline(const LoadPipelineSimple::ArgsTy& args,
		const std::array<std::shared_ptr<SpirvRes>,N>& shaders)
	{
		auto& is_instance_set = std::get<3>(args);
		auto dev = std::get<0>(args);
		auto& extent = std::get<4>(args);
		auto renderPass = std::get<5>(args);

		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (auto& p : shaders)
			push_descriptor(bindings, p->shaderRes.descriptors, p->stage);
		vk::DescriptorSetLayoutCreateInfo info({}, bindings);
		auto descriptorSetLayout = dev.createDescriptorSetLayout(info);

		std::vector<vk::PushConstantRange> pushConsts;
		for (auto& p : shaders)
			push_constant(pushConsts, p->shaderRes.pushConstant, p->stage);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, descriptorSetLayout, pushConsts);
		auto pipelineLayout = dev.createPipelineLayout(pipelineLayoutInfo);

		std::vector<vk::PipelineShaderStageCreateInfo> stages;
		std::vector<vk::ShaderModule> shaderModules;
		for (auto& p : shaders)
		{
			vk::ShaderModuleCreateInfo info({}, p->binary);
			auto sm = dev.createShaderModule(info);
			shaderModules.push_back(sm);
			stages.push_back(vk::PipelineShaderStageCreateInfo({}, p->stage, sm, p->entryPoint.c_str()));
		}
		vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;
		fillPipelineInputState(shaders[0]->shaderRes, vertexInputStateInfo, is_instance_set);
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo({}, vk::PrimitiveTopology::eTriangleList, false);
		vk::Viewport viewport(0.0f,0.0f,extent.width,extent.height,0.0f,1.0f);
		vk::Rect2D scissor({0,0},extent);
		vk::PipelineViewportStateCreateInfo viewportState({},viewport,scissor);
		vk::PipelineMultisampleStateCreateInfo multsampleState({},vk::SampleCountFlagBits::e1,false,1.0f);
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo({},false,false,vk::PolygonMode::eFill,vk::CullModeFlagBits::eBack);
		rasterizationInfo.lineWidth = 1.0f;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState({},true,true,vk::CompareOp::eLessOrEqual,false,true);
		vk::PipelineColorBlendAttachmentState colorBlendAttachment(true,vk::BlendFactor::eSrcAlpha,vk::BlendFactor::eOneMinusSrcAlpha,vk::BlendOp::eAdd,vk::BlendFactor::eOne);
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo({},false,vk::LogicOp::eNoOp,colorBlendAttachment);
		std::vector<vk::DynamicState> dynamicStates = {
			vk::DynamicState::eLineWidth,
			//vk::DynamicState::eStencilOpEXT,
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo dynamicState({},dynamicStates);
		vk::GraphicsPipelineCreateInfo pipelineCreateInfo({},stages, &vertexInputStateInfo, &inputAssemblyStateInfo, nullptr,&viewportState,&rasterizationInfo,
			&multsampleState,&depthStencilState,&colorBlendInfo,&dynamicState,pipelineLayout,renderPass);

		auto pipeline = dev.createGraphicsPipeline({}, pipelineCreateInfo);

		auto data = std::make_shared<PipelineData>();
		data->device = dev;
		data->setLayout = descriptorSetLayout;
		data->pipelineLayout = pipelineLayout;
		data->pipeline = pipeline;
		data->shaderModules = std::move(shaderModules);

		return std::make_tuple(true, data);
	}

	LoadPipelineSimple::RealRetTy LoadPipelineSimple::load(LoadPipelineSimple::ArgsTy args)
	{
		auto vert = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(std::get<1>(args));
		auto frag = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(std::get<2>(args));
		auto dev = std::get<0>(args);
		if (!vert || !frag || !dev)
			return std::make_tuple(false,nullptr);

		std::array<std::shared_ptr<SpirvRes>, 2> shaders = {
			vert,frag
		};

		return realCreatePipeline(args,shaders);
	}
}