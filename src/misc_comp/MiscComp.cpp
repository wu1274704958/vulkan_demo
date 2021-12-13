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

	
}