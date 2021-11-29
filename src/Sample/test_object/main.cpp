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

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 uv;
};
struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},{0.0f,0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f,0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} , {1.0f,1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} ,{0.0f,1.0f}}
};

std::vector<uint16_t> indices = {
	 0, 2, 1, 0, 3, 2
};

struct Texture : public vkd::Component
{
	Texture(std::string path) : path(path)
	{

	}
	void update_descriptor()
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<vkd::PipelineComp>();
		if (pipeline)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorImageInfo image_info(img->sample, img->view, vk::ImageLayout::eShaderReadOnlyOptimal);
			vk::WriteDescriptorSet write_descriptor_set(descStes[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, image_info, {});
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
		update_descriptor();
	}
	void on_clean_up() override
	{
		img.reset();
	}
	void awake() override
	{
		img = gld::DefDataMgr::instance()->load<gld::DataType::VkImage>(path, STBI_rgb_alpha, physical_dev(), device(),
			command_pool(), graphics_queue());
	}
	void draw(vk::CommandBuffer& cmd) override{}
	std::shared_ptr<gld::vkd::VkdImage> img;
	std::string path;
};
class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}

private:
	void onInit() override
	{
		
	}
	void initScene() override
	{
		SampleRender::initScene();
		cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -4.0f));

		scene.lock()->add_child(trans.lock());

		quad = std::make_shared<vkd::Object>("Quad");
		auto quad_t = quad->add_comp<vkd::Transform>();
		quad->add_comp<vkd::Mesh<Vertex,uint16_t>>(vertices,indices);
		quad->add_comp<vkd::PipelineComp>("shader_23/quad.vert", "shader_23/quad.frag");
		quad->add_comp<vkd::DefRender>();
		quad->add_comp<Texture>("textures/texture.jpg");
		scene.lock()->add_child(quad_t.lock());
	}
	std::shared_ptr<vkd::Object> cam_obj,quad;
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


