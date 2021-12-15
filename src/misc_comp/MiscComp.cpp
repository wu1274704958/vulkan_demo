#include <comm_comp/renderpass.hpp>
#include <misc_comp/MiscComp.hpp>

namespace vkd
{
	void Texture::update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		if (pipeline)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info(img->sample, img->view, vk::ImageLayout::eShaderReadOnlyOptimal);
			vk::WriteDescriptorSet write_descriptor_set(descStes[set], binding, 0, vk::DescriptorType::eCombinedImageSampler, image_info, {});
			device().updateDescriptorSets(write_descriptor_set, {});
		}
	}

	bool Texture::on_init()
	{
		update_descriptor();
		return true;
	}

	void Texture::recreate_swapchain()
	{
		update_descriptor();
	}

	void Texture::on_clean_up()
	{
		img.reset();
	}

	void Texture::awake()
	{
		img = gld::DefDataMgr::instance()->load<gld::DataType::VkImage>(path, 4, physical_dev(), device(),
			command_pool(), graphics_queue());
		not_draw = false;
	}

	std::shared_ptr<Component> Texture::clone() const
	{
		return std::make_shared<Texture>(*this);
	}

	DepthSampler::DepthSampler(std::weak_ptr<vkd::OnlyDepthRenderPass> rp, uint16_t set, uint32_t imgBinding, uint32_t samplerBinding)
		: rp(rp),
		set(set),
		imgBinding(imgBinding),
		samplerBinding(samplerBinding)
	{
	}

	DepthSampler::DepthSampler(const DepthSampler& oth)
	{
		this->imgBinding = oth.imgBinding;
		this->samplerBinding = oth.samplerBinding;
		this->set = oth.set;
		this->rp = oth.rp;
	}

	void DepthSampler::awake()
	{
		not_draw = true;
		vk::SamplerCreateInfo info({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::
			SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge
			, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge);
		sampler = device().createSampler(info);
	}

	void DepthSampler::update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		auto renderPass = rp.lock();
		if (pipeline && renderPass)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info({}, renderPass->get_image_view(), vk::ImageLayout::eDepthStencilReadOnlyOptimal);
			vk::DescriptorImageInfo sampler_info(sampler);
			std::array<vk::WriteDescriptorSet, 2> descriptor_sets = {
				vk::WriteDescriptorSet(descStes[set], imgBinding, 0, vk::DescriptorType::eSampledImage, image_info, {}),
				vk::WriteDescriptorSet(descStes[set], samplerBinding, 0, vk::DescriptorType::eSampler, sampler_info, {})
			};
			device().updateDescriptorSets(descriptor_sets, {});
		}
	}

	bool DepthSampler::on_init()
	{
		update_descriptor();
		return true;
	}
	void DepthSampler::recreate_swapchain()
	{
		update_descriptor();
	}
	void DepthSampler::on_clean_up()
	{
		device().destroySampler(sampler);
	}
	std::shared_ptr<Component> DepthSampler::clone() const
	{
		return std::make_shared<DepthSampler>(*this);
	}
}