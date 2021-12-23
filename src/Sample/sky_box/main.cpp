#include <core/object.hpp>
#include <comm_comp/transform.hpp>
#include <sample/render.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <glm/glm.hpp>
#include <comm_comp/showcase.hpp>
#include <comm_comp/scene.hpp>
#include <generator/Generator.hpp>
#include <sundry.hpp>
#include <comm_comp/sky_box.hpp>

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
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -1.0f));

		main_scene.lock()->add_child(trans.lock());

		auto cube_obj = std::make_shared<vkd::Object>("Skybox");
		auto cube_trans = cube_obj->add_comp<vkd::Transform>();
		cube_obj->add_comp<vkd::SkyBox>("skybox/skybox.json");

		main_scene.lock()->add_child(cube_trans.lock());
	}
private:
};

#include <event/event.hpp>
int main()
{
	gld::DefResMgr::create_instance(std::make_tuple("../../../res"));
	auto quad = new Quad(true, "SkyBox");
	quad->init(800, 600);
	quad->mainLoop();
	gld::DefDataMgr::instance()->clear_all();
	quad->cleanUp();
	delete quad;
	return 0;
}


