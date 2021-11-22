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
		auto cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -4.0f));

		scene.lock()->add_child(trans.lock());
	}
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

