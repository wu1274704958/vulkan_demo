#include <comm_comp/mesh.hpp>
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
		load_image();
		not_draw = false;
	}

	void Texture::load_image()
	{
		img = gld::DefDataMgr::instance()->load<gld::DataType::VkImage>(path, 4, physical_dev(), device(),
			command_pool(), graphics_queue());
	}


	std::shared_ptr<Component> Texture::clone() const
	{
		return std::make_shared<Texture>(*this);
	}

	std::shared_ptr<Component> TextureArray::clone() const
	{
		return std::make_shared<TextureArray>(*this);
	}

	void TextureArray::load_image()
	{
		img = gld::DefDataMgr::instance()->load<gld::DataType::VkImageArray>(path, 4, physical_dev(), device(),
			command_pool(), graphics_queue());
	}

	std::shared_ptr<Component> TextureCube::clone() const
	{
		return std::make_shared<TextureCube>(*this);
	}

	void TextureCube::load_image()
	{
		img = gld::DefDataMgr::instance()->load<gld::DataType::VkImageCube>(path, 4, physical_dev(), device(),
			command_pool(), graphics_queue());
	}

	DepthSampler::DepthSampler(std::weak_ptr<vk::ImageView> imgView, uint16_t set, uint32_t imgBinding, uint32_t samplerBinding)
		: imgView(imgView),
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
		this->imgView = oth.imgView;
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
		auto img_view = imgView.lock();
		if (pipeline && img_view)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info({}, *img_view, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
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


	bool RenderOrigin::on_init()
	{
		auto obj = object.lock();
		mesh = obj->get_comp_dyn<vkd::MeshInterface>();
		return !mesh.expired();
	}
	void RenderOrigin::draw(vk::CommandBuffer& cmd)
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		auto mesh_ptr = mesh.lock();

		if (pipeline && mesh_ptr)
		{
			cmd.drawIndexed(mesh_ptr->index_count(), 1, 0, 0, 0);
		}
	}

	std::shared_ptr<Component> RenderOrigin::clone() const
	{
		return std::make_shared<RenderOrigin>(*this);
	}

	ScreenQuad::ScreenQuad() : Mesh(Vertices, Indices, "_SYS_ScreenQuadVertices") {}

	std::shared_ptr<std::vector<glm::vec4>> ScreenQuad::Vertices = std::make_shared<std::vector<glm::vec4>>(std::vector<glm::vec4>{
		{-1.f, -1.f, 0.0f, 0.0f},
		{ 1.f, -1.f,1.0f,0.0f },
		{ 1.f,  1.f,1.0f,1.0f },
		{ -1.f,  1.f,0.0f,1.0f }}
	);

	std::shared_ptr<std::vector<uint16_t>> ScreenQuad::Indices = std::make_shared<std::vector<uint16_t>>(std::vector<uint16_t>{
		0, 2, 1, 0, 3, 2});

	ImageComp::ImageComp(std::weak_ptr<vk::ImageView> imgView,vk::ImageLayout layout, uint16_t set, uint32_t imgBinding)
		: imgView(imgView), set(set),imgBinding(imgBinding),layout(layout)
	{
		
	}
	
	std::shared_ptr<Component> ImageComp::clone() const
	{
		return std::make_shared<ImageComp>(*this);
	}

	void ImageComp::awake()
	{
		not_draw = true;
	}

	void ImageComp::update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		auto img_view = imgView.lock();
		if (pipeline && img_view)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info({}, *img_view, layout);
			std::array<vk::WriteDescriptorSet, 1> descriptor_sets = {
				vk::WriteDescriptorSet(descStes[set], imgBinding, 0, vk::DescriptorType::eSampledImage, image_info, {}),
			};
			device().updateDescriptorSets(descriptor_sets, {});
		}
	}

	bool ImageComp::on_init()
	{
		update_descriptor();
		return true;
	}
	void ImageComp::recreate_swapchain()
	{
		update_descriptor();
	}
	void ImageComp::on_clean_up()
	{
	}


}