#include <core/object.hpp>

#include <comm_comp/transform.hpp>
#include <comm_comp/pipeline.hpp>
#include <sample/render.hpp>
#include <json.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <comm_comp/showcase.hpp>
#include <comm_comp/scene.hpp>
#include <generator/Generator.hpp>
#include <comm_comp/render.hpp>
#include <misc_comp/MiscComp.hpp>
#include <sundry.hpp>


struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 uv;
};
struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

std::vector<Vertex> Vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},{0.0f,0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} , {1.0f,1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} ,{0.0f,1.0f}}
};
std::vector<Vertex> DepVertices = {
	{{-1.f, -1.f}, {1.0f, 0.0f, 0.0f},{0.0f,0.0f}},
	{{ 1.f, -1.f}, {0.0f, 1.0f, 0.0f}, {1.0f,0.0f}},
	{{ 1.f,  1.f}, {0.0f, 0.0f, 1.0f} , {1.0f,1.0f}},
	{{-1.f,  1.f}, {1.0f, 1.0f, 1.0f} ,{0.0f,1.0f}}
};
std::vector<uint16_t> Indices = {
	 0, 2, 1, 0, 3, 2
};

struct ScreenDraw : public vkd::Component
{
	bool on_init() override
	{
		auto obj = object.lock();
		mesh = obj->get_comp_dyn<vkd::MeshInterface>();
		return !mesh.expired();
	}
	int64_t idx() override { return std::numeric_limits<int64_t>::max() - 1; }
	void draw(vk::CommandBuffer& cmd) override
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		auto mesh_ptr = mesh.lock();

		if (pipeline && mesh_ptr)
		{
			cmd.drawIndexed(mesh_ptr->index_count(), 1, 0, 0, 0);
		}
	}
	void on_clean_up() override{}
protected:
	std::weak_ptr<vkd::MeshInterface> mesh;
	
};

struct DepthSampler : public vkd::Component
{
	DepthSampler(uint16_t set = 0, uint32_t binding = 1)
		: set(set),
		  binding(binding)
	{
	}

	void awake() override
	{
		not_draw = true;
		vk::SamplerCreateInfo info({});
		sampler = device().createSampler(info);
		createDepthImage();
		copyToDepthImage();
	}
	void update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		if (pipeline)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info(sampler, depthView, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
			vk::WriteDescriptorSet write_descriptor_set(descStes[set], binding, 0, vk::DescriptorType::eCombinedImageSampler, image_info, {});
			device().updateDescriptorSets(write_descriptor_set, {});
		}
	}
	bool on_init() override
	{
		update_descriptor();
		return true;
	}
	void recreate_swapchain() override
	{
		createDepthImage();
		update_descriptor();
	}
	void on_clean_up() override
	{
		device().destroySampler(sampler);
	}

	void clean_up_pipeline() override
	{
		auto d = device();
		d.freeMemory(depthMem);
		d.destroyImageView(depthView);
		d.destroyImage(depthImg);
	}
public:
	void createDepthImage()
	{
		auto sur_extent = surface_extent();
		sundry::createImage(physical_dev(), device(), sur_extent.width, sur_extent.height, depthstencil_format(), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst |
			vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImg, depthMem, nullptr);
		
		vk::ImageViewCreateInfo infoView({}, depthImg, vk::ImageViewType::e2D, depthstencil_format(), vk::ComponentMapping(),
			vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1));
		depthView = device().createImageView(infoView);
	}
	void copyToDepthImage(bool copy = false)
	{
		auto sur_extent = surface_extent();
		vk::CommandBuffer cmd = sundry::beginSingleTimeCommands(device(), command_pool());
		vk::ImageMemoryBarrier barrier;
		barrier.image = depthImg;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1);
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0, {}, {}, barrier);
		if(copy)
		{
			vk::ImageCopy regions;
			regions.srcOffset = vk::Offset3D(0,0,0);
			regions.dstOffset = regions.srcOffset;
			regions.extent = vk::Extent3D(sur_extent, 1);
			regions.srcSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 1, 0, 1);
			regions.dstSubresource = regions.srcSubresource;
			cmd.copyImage(depth_attachment().image, vk::ImageLayout::eDepthStencilAttachmentOptimal,
				depthImg, vk::ImageLayout::eDepthStencilReadOnlyOptimal, regions);
		}
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNoneKHR;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, (vk::DependencyFlagBits)0, {}, {}, barrier);
		sundry::endSingleTimeCommands(device(), cmd, command_pool(), graphics_queue());
	}
protected:
	vk::Sampler sampler;
	vk::Image depthImg;
	vk::ImageView depthView;
	vk::DeviceMemory depthMem;
	uint16_t set;
	uint32_t binding;
};


class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}

private:
	void onInit() override
	{
		vertices = std::make_shared<std::vector<Vertex>>(Vertices);
		indices = std::make_shared<std::vector<uint16_t>>(Indices);
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent,
			"shader_23/instance.vert", "shader_23/instance.frag", 1, std::unordered_set<uint32_t>{1},std::vector<uint32_t>{3});
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent,
			"shader_23/depth.vert", "shader_23/depth.frag");

		prepare_instance();
	}
	void initScene() override
	{
		SampleRender::initScene();
		auto cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -12.0f));

		scene.lock()->add_child(trans.lock());

		quad = std::make_shared<vkd::Object>("Quad");
		auto quad_t = quad->add_comp<vkd::Transform>();
		quad->add_comp<vkd::Mesh<Vertex,uint16_t>>(vertices,indices);
		quad->add_comp<vkd::PipelineComp>("shader_23/instance.vert", "shader_23/instance.frag");
		quad->add_comp<vkd::MeshInstance<glm::mat4>>(instanceData);
		quad->add_comp<vkd::DefRenderInstance>();
		quad->add_comp<vkd::Texture>("textures/texture.jpg");
		quad->add_comp<vkd::ViewportScissor>(glm::vec4(0.f,0.f,1.f,1.f), glm::vec4(0.f, 0.f, 1.f, 1.0f));
		scene.lock()->add_child(quad_t.lock());

		quad2 = std::make_shared<vkd::Object>("Quad2");
		auto quad2_t = quad2->add_comp<vkd::Transform>();
		quad2->add_comp<vkd::Mesh<Vertex, uint16_t>>(std::make_shared<std::vector<Vertex>>(DepVertices), indices);
		quad2->add_comp<vkd::PipelineComp>("shader_23/depth.vert", "shader_23/depth.frag");
		quad2->add_comp<ScreenDraw>();
		quad2->add_comp<vkd::Texture>("textures/texture.jpg");
		//quad2->add_comp<DepthSampler>();
		quad2->add_comp<vkd::ViewportScissor>(glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.5f, 0.f, 0.5f, 1.0f));
		scene.lock()->add_child(quad2_t.lock());
	}

	void prepare_instance()
	{
		int w = 10,h = 10,d = 10;
		float space = 0.5f,size = 1.0f;
		float bx = -((w - 1) * size + (w - 1) * space) / 2.0f;
		float by = -((h - 1) * size + (h - 1) * space) / 2.0f;
		float bz = -((d - 1) * size + (d - 1) * space) / 2.0f;

		instanceData = std::make_shared<std::vector<glm::mat4>>();
		for(int z = 0;z < d;++z)
		{
			for (int y = 0; y < h; ++y)
			{
				for (int x = 0; x < w; ++x)
				{
					glm::mat4 mat(1.0f);
					mat = glm::translate(mat,glm::vec3(bx,by,bz));
					instanceData->push_back(mat);
					bx += (size + space);
				}
				bx = -((w - 1) * size + (w - 1) * space) / 2.0f;
				by += (size + space);
			}
			bx = -((w - 1) * size + (w - 1) * space) / 2.0f;
			by = -((h - 1) * size + (h - 1) * space) / 2.0f;
			bz += (size + space);
		}
	}

	void onReCreateSwapChain() override
	{
		vkd::SampleRender::onReCreateSwapChain();
		quad->get_comp_raw<vkd::ViewportScissor>()->reset(glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.0f));
		quad2->get_comp_raw<vkd::ViewportScissor>()->reset(glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.5f, 0.f, 0.5f, 1.0f));
	}
private:
	std::shared_ptr<vkd::Object> quad,quad2;
	std::shared_ptr<std::vector<Vertex>> vertices;
	std::shared_ptr<std::vector<uint16_t>> indices;
	std::shared_ptr<std::vector<glm::mat4>> instanceData;
};

#include <event/event.hpp>
int main()
{
	gld::DefResMgr::create_instance(std::make_tuple("../../../res"));
	auto quad = new Quad(true, "Quad");
	quad->init(800, 600);
	quad->mainLoop();
	gld::DefDataMgr::instance()->clear_all();
	quad->cleanUp();
	delete quad;
	return 0;
}


