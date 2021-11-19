#include <data_pipeline.hpp>
#include <sundry.hpp>
#include <res_loader/resource_mgr.hpp>
#include <common.hpp>

namespace gld::vkd {

	std::string LoadPipelineSimpleTy::key_from_args(
		vk::Device, vk::RenderPass, const vk::Extent2D&,const std::string& v,const std::string& f,uint32_t, const std::unordered_set<uint32_t>& ins_set, std::function<void(vk::GraphicsPipelineCreateInfo)>
		)
	{
		return sundry::format_tup('#',v,f);
	}
	std::string LoadPipelineSimpleTy::key_from_args(
		const std::string& v, const std::string& f
	)
	{
		return sundry::format_tup('#', v, f);
	}
	inline void push_descriptor(std::vector<std::vector<vk::DescriptorSetLayoutBinding>>& bindings,std::vector<Descriptor>& descriptors,vk::ShaderStageFlagBits stage)
	{
		for (auto& d : descriptors)
		{
			vk::DescriptorSetLayoutBinding binding;
			binding.binding = d.binding;
			binding.descriptorCount = 1;
			binding.descriptorType = d.type;
			binding.stageFlags = stage;
			if(bindings.size() <= d.set)
				bindings.resize(d.set + 1);
			bindings[d.set].push_back(binding);
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
	template<size_t N>
	inline void push_descriptor_pool_size(std::vector<vk::DescriptorPoolSize>& poolSizes, const std::array<std::shared_ptr<SpirvRes>, N>& shaders)
	{
		std::unordered_map<vk::DescriptorType,uint32_t> map;	
		for(auto a : shaders)
		{
			for (auto& d : a->shaderRes.descriptors)
			{
				if (map.contains(d.type))
				{
					map[d.type] += 1;
				}
				else {
					map.insert(std::make_pair(d.type,1));
				}
			}
		}
		for (auto& a : map)
		{
			poolSizes.push_back(vk::DescriptorPoolSize(a.first,a.second));
		}
	}
	
	inline std::tuple<std::shared_ptr<std::vector<vk::VertexInputBindingDescription>>, std::shared_ptr<std::vector<vk::VertexInputAttributeDescription>>>
		fillPipelineInputState(ShaderResources& res, vk::PipelineVertexInputStateCreateInfo& inputState,const std::unordered_set<uint32_t>& is_instance_set)
	{
		std::shared_ptr<std::vector<vk::VertexInputBindingDescription>> binding = std::make_shared<std::vector<vk::VertexInputBindingDescription>>();
		for (auto& r : res.inBindingStride)
		{
			binding->push_back(vk::VertexInputBindingDescription(r.binding,r.stride,
				is_instance_set.contains(r.binding) ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex));
		}
		std::shared_ptr<std::vector<vk::VertexInputAttributeDescription>> attr = std::make_shared<std::vector<vk::VertexInputAttributeDescription>>();
		for (auto& r : res.stageInput)
		{
			attr->push_back(vk::VertexInputAttributeDescription(r.location,r.binding,r.format,r.offset));
		}
		
		inputState.setVertexBindingDescriptions(*binding);
		inputState.setVertexAttributeDescriptions(*attr);
		return std::make_tuple(binding,attr);
	}

	template<size_t N>
	inline vk::DescriptorPool createDescriptorPool(vk::Device dev, const std::array<std::shared_ptr<SpirvRes>, N>& shaders, uint32_t maxPoolSize)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		push_descriptor_pool_size(poolSizes, shaders);
		vk::DescriptorPoolCreateInfo info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,maxPoolSize,poolSizes);
		return dev.createDescriptorPool(info);
	}

	template<size_t N>
	LoadPipelineSimpleTy::RealRetTy 
		realCreatePipeline(vk::Device dev, vk::RenderPass renderPass, const vk::Extent2D& extent,const std::unordered_set<uint32_t>& is_instance_set, std::function<void(vk::GraphicsPipelineCreateInfo)>& on,
		const std::array<std::shared_ptr<SpirvRes>,N>& shaders, uint32_t maxPoolSize)
	{
		std::vector<std::vector<vk::DescriptorSetLayoutBinding>> bindings;
		for (auto& p : shaders)
			push_descriptor(bindings, p->shaderRes.descriptors, p->stage);
		vk::DescriptorSetLayoutCreateInfo info;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
		for (auto& b : bindings)
		{
			info.setBindings(b);
			descriptorSetLayout.push_back(dev.createDescriptorSetLayout(info));
		}

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
		auto _ = fillPipelineInputState(shaders[0]->shaderRes, vertexInputStateInfo, is_instance_set);
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo({}, vk::PrimitiveTopology::eTriangleList, false);
		vk::Viewport viewport(0.0f,0.0f,extent.width,extent.height,0.0f,1.0f);
		vk::Rect2D scissor({0,0},extent);
		vk::PipelineViewportStateCreateInfo viewportState({},viewport,scissor);
		vk::PipelineMultisampleStateCreateInfo multsampleState({},vk::SampleCountFlagBits::e1,false,1.0f);
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo({},false,false,vk::PolygonMode::eFill,vk::CullModeFlagBits::eBack);
		rasterizationInfo.lineWidth = 1.0f;
		vk::PipelineDepthStencilStateCreateInfo depthStencilState({},true,true,vk::CompareOp::eLessOrEqual,false,false);
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
		if(on)on(pipelineCreateInfo);
		auto pipeline = dev.createGraphicsPipeline({}, pipelineCreateInfo);
		auto data = std::make_shared<PipelineData>();
		data->device = dev;

		data->setLayout = descriptorSetLayout;
		data->pipelineLayout = pipelineLayout;
		data->pipeline = pipeline;
		data->shaderModules = std::move(shaderModules);
		data->descriptorPool = createDescriptorPool<N>(dev,shaders,maxPoolSize * descriptorSetLayout.size());

		return std::make_tuple(true, data);
	}

	LoadPipelineSimpleTy::RealRetTy 
		LoadPipelineSimpleTy::load(
		vk::Device dev, vk::RenderPass r, const vk::Extent2D& extent, std::string vert_s, std::string frag_s,uint32_t maxPoolSize, std::unordered_set<uint32_t> is_ins, std::function<void(vk::GraphicsPipelineCreateInfo)> on
	)
	{
		auto vert = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(vert_s);
		auto frag = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(frag_s);
		if (!vert || !frag || !dev)
			return std::make_tuple(false,nullptr);

		std::array<std::shared_ptr<SpirvRes>, 2> shaders = {
			vert,frag
		};

		return realCreatePipeline(dev,r,extent,is_ins,on,shaders,maxPoolSize);
	}

	PipelineData::~PipelineData()
	{
		if (!device) return;
		if (pipeline) device.destroyPipeline(pipeline);
		for (auto r : shaderModules)
		{
			if (r) device.destroyShaderModule(r);
		}
		if (descriptorPool) device.destroyDescriptorPool(descriptorPool);
		if (pipelineLayout) device.destroyPipelineLayout(pipelineLayout);
		for (auto s : setLayout)
			if (s) device.destroyDescriptorSetLayout(s);
	}
	std::vector<vk::DescriptorSet> PipelineData::allocDescriptorSets()
	{
		vk::DescriptorSetAllocateInfo info(descriptorPool, setLayout);
		return device.allocateDescriptorSets(info);
	}
}
