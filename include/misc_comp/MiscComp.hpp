#pragma once

#include <string>
#include <core/object.hpp>
#include <core/component.hpp>
#include <comm_comp/pipeline.hpp>
#include <res_loader/data_mgr.hpp>

namespace vkd
{
	struct Texture : public vkd::Component
	{
		Texture(std::string path,uint16_t set = 0,uint32_t binding = 1) : path(path),set(set),binding(binding){}
		void update_descriptor() const;
		bool on_init() override;
		void recreate_swapchain() override;
		void on_clean_up() override;
		void awake() override;
		std::shared_ptr<Component> clone() const override;
		virtual void load_image();
	protected:
		std::shared_ptr<gld::vkd::VkdImage> img;
		std::string path;
		uint16_t set;
		uint32_t binding;
	};

	struct TextureArray : public Texture
	{
		TextureArray(std::string path, uint16_t set = 0, uint32_t binding = 1) : Texture(path,set,binding) {}
		void load_image() override;
		std::shared_ptr<Component> clone() const override;
	};

	struct TextureCube : public Texture
	{
		TextureCube(std::string path, uint16_t set = 0, uint32_t binding = 1) : Texture(path, set, binding) {}
		void load_image() override;
		std::shared_ptr<Component> clone() const override;
	};

	struct OnlyDepthRenderPass;

	struct DepthSampler : public vkd::Component
	{
		DepthSampler(std::weak_ptr<vk::ImageView> imgView, uint16_t set = 0, uint32_t imgBinding = 1, uint32_t samplerBinding = 2);
		DepthSampler(const DepthSampler& oth);

		void awake() override;
		void update_descriptor() const;
		bool on_init() override;
		void recreate_swapchain() override;
		void on_clean_up() override;
		std::shared_ptr<Component> clone() const override;

	protected:
		std::weak_ptr< vk::ImageView > imgView;
		vk::Sampler sampler;
		uint16_t set;
		uint32_t imgBinding, samplerBinding;
	};

	struct RenderOrigin : public vkd::Component
	{
		bool on_init() override;
		int64_t idx() override { return std::numeric_limits<int64_t>::max() - 1; }
		void draw(vk::CommandBuffer& cmd) override;
		void on_clean_up() override {}
		std::shared_ptr<Component> clone() const override;
	protected:
		std::weak_ptr<vkd::MeshInterface> mesh;
	};

	struct ScreenQuad: public Mesh<glm::vec4,uint16_t>
	{
		ScreenQuad();
		static std::shared_ptr<std::vector<glm::vec4>> Vertices;
		static std::shared_ptr<std::vector<uint16_t>> Indices;
	};

	struct ImageComp : public vkd::Component
	{
		ImageComp(std::weak_ptr<vk::ImageView> imgView, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal, uint16_t set = 0, uint32_t imgBinding = 1);
		ImageComp(const ImageComp& oth) = default;

		void awake() override;
		void update_descriptor() const;
		bool on_init() override;
		void recreate_swapchain() override;
		void on_clean_up() override;
		std::shared_ptr<Component> clone() const override;

	protected:
		std::weak_ptr< vk::ImageView > imgView;
		uint16_t set;
		uint32_t imgBinding, samplerBinding;
		vk::ImageLayout layout;
	};
}
