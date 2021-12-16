#include <sundry.hpp>
#include  <comm_comp/renderpass.hpp>
#include <comm_comp/scene.hpp>
#include <Sample/render.hpp>

namespace vkd
{
	void DefRenderPass::awake()
	{
		m_render_pass = create_renderpass();
	}

	bool DefRenderPass::on_init()
	{
		const auto obj = object.lock();
		auto scene = obj->get_comp_raw<Scene>();
		scene->set_renderpass(m_render_pass);
		return true;
	}

	void DefRenderPass::pre_draw(vk::CommandBuffer& cmd)
	{
		renderpass_begin(cmd,cnt);
	}

	void DefRenderPass::after_draw(vk::CommandBuffer& cmd)
	{
		cmd.endRenderPass();
	}

	vk::RenderPass DefRenderPass::getRenderPass() const
	{
		return m_render_pass;
	}

	vk::RenderPass DefRenderPass::create_renderpass()
	{
		return renderpass();
	}

	void DefRenderPass::renderpass_begin(vk::CommandBuffer& cmd,vk::SubpassContents cnt)
	{
		cmd.beginRenderPass(render_pass_begin_info(), cnt);
	}

	void DefRenderPass::recreate_swapchain()
	{
		awake();
		on_init();
	}

	std::shared_ptr<Component> DefRenderPass::clone() const
	{
		return std::make_shared<DefRenderPass>(*this);
	}

	void OnlyDepthRenderPass::awake()
	{
		create_depth_attachment();
		DefRenderPass::awake();
	}

	void OnlyDepthRenderPass::create_depth_attachment()
	{
		const auto& surfaceExtent = surface_extent();
		auto format = depthstencil_format();
		auto dev = device();
		vk::ImageCreateInfo imgInfo({}, vk::ImageType::e2D,format , vk::Extent3D(surfaceExtent.width, surfaceExtent.height, 1), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, {});
		depth = dev.createImage(imgInfo);
		auto req = dev.getImageMemoryRequirements(depth);

		vk::MemoryAllocateInfo allocInfo(req.size, sundry::findMemoryType(physical_dev(),req.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));
		mem = dev.allocateMemory(allocInfo);

		dev.bindImageMemory(depth, mem, 0);

		vk::ImageViewCreateInfo viewInfo({}, depth, vk::ImageViewType::e2D, format, {},
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
		*view = dev.createImageView(viewInfo);
	}

	vk::RenderPass OnlyDepthRenderPass::create_renderpass()
	{
		const auto& surfaceExtent = surface_extent();
		auto format = depthstencil_format();
		auto dev = device();
		const auto& depthAttachment = depth_attachment();
		std::vector<vk::AttachmentDescription> attachment = {
			vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),format,vk::SampleCountFlagBits::e1,vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eClear,vk::AttachmentStoreOp::eStore,vk::ImageLayout::eUndefined,vk::ImageLayout::eDepthStencilReadOnlyOptimal)
		};

		vk::AttachmentReference depthAttachmentRef(0, vk::ImageLayout::eDepthStencilAttachmentOptimal);

		std::array<vk::SubpassDescription, 1> subpassDesc = {};
		subpassDesc[0] = vk::SubpassDescription();
		subpassDesc[0].setPDepthStencilAttachment(&depthAttachmentRef);
		subpassDesc[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

		vk::RenderPassCreateInfo info(vk::RenderPassCreateFlags(), attachment, subpassDesc);
		auto renderPass = dev.createRenderPass(info);

		vk::FramebufferCreateInfo fbinfo({}, renderPass, *view, surfaceExtent.width, surfaceExtent.height, 1);
	
		framebuffer = dev.createFramebuffer(fbinfo);

		return renderPass;
	}

	void OnlyDepthRenderPass::renderpass_begin(vk::CommandBuffer& cmd, vk::SubpassContents cnt)
	{
		const auto& surfaceExtent = surface_extent();
		auto clearVals = vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0));
		vk::RenderPassBeginInfo info(m_render_pass,framebuffer, vk::Rect2D({ 0,0 }, surfaceExtent), clearVals);
		cmd.beginRenderPass(info,cnt);
	}

	vk::ImageLayout OnlyDepthRenderPass::get_image_layout() const
	{
		return depth_attachment().imgLayout;
	}

	std::weak_ptr<vk::ImageView> OnlyDepthRenderPass::get_depth_image_view() const
	{
		return view;
	}
	
	void OnlyDepthRenderPass::clean_up_pipeline()
	{
		auto dev = device();
		if(mem)dev.freeMemory(mem);
		if(*view)dev.destroyImageView(*view);
		if(depth)dev.destroyImage(depth);
		if(framebuffer)dev.destroyFramebuffer(framebuffer);
		if(m_render_pass)dev.destroyRenderPass(m_render_pass);
	}

	std::shared_ptr<Component> OnlyDepthRenderPass::clone() const 
	{
		return std::make_shared<OnlyDepthRenderPass>(*this);
	}

	OnlyDepthRenderPass::OnlyDepthRenderPass(const OnlyDepthRenderPass& oth)
	{
	}

	void OnlyDepthRenderPass::on_destroy()
	{
		view.reset();
	}



}

