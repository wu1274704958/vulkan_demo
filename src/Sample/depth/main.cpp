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
#include <comm_comp/renderpass.hpp>


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
	DepthSampler(std::weak_ptr<vkd::OnlyDepthRenderPass> rp,uint16_t set = 0, uint32_t imgBinding = 1,uint32_t samplerBinding = 2)
		: rp(rp),
		set(set),
		imgBinding(imgBinding),
		samplerBinding(samplerBinding)
	{
	}

	void awake() override
	{
		not_draw = true;
		vk::SamplerCreateInfo info({},vk::Filter::eLinear, vk::Filter::eLinear, vk::
		SamplerMipmapMode::eLinear,vk::SamplerAddressMode::eClampToEdge
		, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge);
		sampler = device().createSampler(info);
	}
	void update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		auto renderPass = rp.lock();
		if (pipeline && renderPass)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info({}, renderPass->get_image_view(),vk::ImageLayout::eDepthStencilReadOnlyOptimal);
			vk::DescriptorImageInfo sampler_info(sampler);
			std::array<vk::WriteDescriptorSet,2> descriptor_sets = {
				vk::WriteDescriptorSet(descStes[set], imgBinding, 0, vk::DescriptorType::eSampledImage, image_info, {}),
				vk::WriteDescriptorSet(descStes[set], samplerBinding, 0, vk::DescriptorType::eSampler, sampler_info, {})
			};
			device().updateDescriptorSets(descriptor_sets, {});
		}
	}
	bool on_init() override
	{
		update_descriptor();
		return true;
	}
	void recreate_swapchain() override
	{
		update_descriptor();
	}
	void on_clean_up() override
	{
		device().destroySampler(sampler);
	}
	
protected:
	std::weak_ptr<vkd::OnlyDepthRenderPass> rp;
	vk::Sampler sampler;
	uint16_t set;
	uint32_t imgBinding,samplerBinding;
};


class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}

private:
	void onInit() override
	{
		vertices = std::make_shared<std::vector<Vertex>>(Vertices);
		indices = std::make_shared<std::vector<uint16_t>>(Indices);

		prepare_instance();
	}
	void initScene() override
	{
		SampleRender::initScene();
		auto cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -12.0f));

		auto cam2_obj = std::make_shared<vkd::Object>("Camera2");
		auto cam2_trans = cam2_obj->add_comp<vkd::Transform>();
		cam2_obj->add_comp<vkd::Showcase>();
		cam2_trans.lock()->set_position(glm::vec3(0.f, 0.f, -12.0f));
		
		main_scene.lock()->add_child(trans.lock());

		auto quad_t = createQuad();
		main_scene.lock()->add_child(quad_t->get_comp<vkd::Transform>().lock());

		auto mscene = main_scene_obj.lock();
		mscene->destroy_comp<vkd::DefRenderPass>();
		auto depthComp = mscene->add_comp<vkd::OnlyDepthRenderPass>();

		auto scene_obj = std::make_shared<vkd::Object>("Real");
		main_scene_obj = scene_obj;
		main_scene = scene_obj->add_comp<vkd::Scene>();
		scene_obj->add_comp<vkd::DefRenderPass>();
		main_scene.lock()->add_child(cam2_trans.lock());
		addScene(scene_obj);

		quad1 = std::make_shared<vkd::Object>("Quad2");
		auto quad1_t = quad1->add_comp<vkd::Transform>();
		quad1->add_comp<vkd::Mesh<Vertex, uint16_t>>(std::make_shared<std::vector<Vertex>>(DepVertices), indices);
		quad1->add_comp<vkd::PipelineComp>("shader_23/depth.vert", "shader_23/depth.frag");
		quad1->add_comp<ScreenDraw>();
		//quad1->add_comp<vkd::Texture>("textures/texture.jpg");
		quad1->add_comp<DepthSampler>(depthComp);
		quad1->add_comp<vkd::ViewportScissor>(glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.5f, 0.f, 0.5f, 1.0f));

		quad3 = createQuad();
		quad3->add_comp<vkd::ViewportScissor>(glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 0.5f, 1.0f));

		main_scene.lock()->add_child(quad3->get_comp<vkd::Transform>().lock());
		main_scene.lock()->add_child(quad1_t.lock());
	}

	std::shared_ptr<vkd::Object> createQuad()
	{
		auto quad = std::make_shared<vkd::Object>("Quad");
		auto quad_t = quad->add_comp<vkd::Transform>();
		quad->add_comp<vkd::Mesh<Vertex, uint16_t>>(vertices, indices);
		quad->add_comp<vkd::PipelineComp>("shader_23/instance.vert", "shader_23/instance.frag", 1, std::unordered_set<uint32_t>{1}, std::vector<uint32_t>{3});
		quad->add_comp<vkd::MeshInstance<glm::mat4>>(instanceData);
		quad->add_comp<vkd::DefRenderInstance>();
		quad->add_comp<vkd::Texture>("textures/texture.jpg");
		return quad;
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
	void onCleanUp() override
	{
		quad1.reset();
		quad3.reset();
		vkd::SampleRender::onCleanUp();
	}
private:
	std::shared_ptr<vkd::Object> quad1, quad3;
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


