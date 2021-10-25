#include <data_pipeline.hpp>
#include <sundry.hpp>
#include <res_loader/resource_mgr.hpp>
namespace gld::vkd {

	std::string LoadPipelineSimple::key_from_args(const ArgsTy& args)
	{
		std::string s = std::get<1>(args);
		s += '#';
		s += std::get<2>(args);
		return s;
	}
	void push_descriptor(std::vector<vk::DescriptorSetLayoutBinding>& bindings,std::vector<Descriptor>& descriptors,vk::ShaderStageFlagBits stage)
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
	LoadPipelineSimple::RealRetTy LoadPipelineSimple::load(LoadPipelineSimple::ArgsTy args)
	{
		auto vert = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(std::get<1>(args));
		auto frag = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>(std::get<2>(args));
		auto dev = std::get<0>(args);
		if (!vert || !frag || !dev)
			return std::make_tuple(false,nullptr);

		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		push_descriptor(bindings,vert->shaderRes.descriptors,vert->stage);
		push_descriptor(bindings, frag->shaderRes.descriptors, frag->stage);
		vk::DescriptorSetLayoutCreateInfo info({},bindings);
		auto descriptorSetLayout = dev.createDescriptorSetLayout(info);

		std::vector<vk::PushConstantRange> pushConsts;
		for (auto& r : vert->shaderRes.pushConstant)
		{
			vk::PushConstantRange;
			pushConsts.push_back()
		}
		
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo({},descriptorSetLayout,pushConsts);
		auto pipelineLayout = dev.createPipelineLayout(pipelineLayoutInfo);

		return std::make_tuple(false, nullptr);
	}
}