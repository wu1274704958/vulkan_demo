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
	glm::vec3 pos;
	glm::vec3 uv;
};

#define HALF 0.5f

std::vector<Vertex> Vertices = {
	{ {  HALF, HALF, HALF},{0.0f,0.0f,4.0f} }, //0 前左上
	{ {  HALF,-HALF, HALF},{0.0f,1.0f,4.0f} }, //1 前左下
	{ { -HALF,-HALF, HALF},{1.0f,1.0f,4.0f} }, //2 前右下
	{ { -HALF, HALF, HALF},{1.0f,0.0f,4.0f} }, //3 前右上

	{ {  HALF, HALF, -HALF},{0.0f,0.0f,2.0f} }, //4 后左上
	{ {  HALF,-HALF, -HALF},{0.0f,1.0f,2.0f} }, //5 后左下
	{ { -HALF,-HALF, -HALF},{1.0f,1.0f,2.0f} }, //6 后右下
	{ { -HALF, HALF, -HALF},{1.0f,0.0f,2.0f} }, //7 后右上

	{ {  HALF, HALF, -HALF},{0.0f,0.0f,0.0f} }, //8 上左上
	{ {  HALF, HALF,  HALF},{0.0f,1.0f,0.0f} }, //9 上左下
	{ { -HALF, HALF,  HALF},{1.0f,1.0f,0.0f} }, //10 上右下
	{ { -HALF, HALF, -HALF},{1.0f,0.0f,0.0f} }, //11 上右上

	{ {  HALF,-HALF, -HALF},{0.0f,0.0f,5.0f} }, //12 下左上
	{ {  HALF,-HALF,  HALF},{0.0f,1.0f,5.0f} }, //13 下左下
	{ { -HALF,-HALF,  HALF},{1.0f,1.0f,5.0f} }, //14 下右下
	{ { -HALF,-HALF, -HALF},{1.0f,0.0f,5.0f} }, //15 下右上

	{ {  HALF, HALF, -HALF},{0.0f,0.0f,1.0f} }, //16 左 左上
	{ {  HALF,-HALF, -HALF},{0.0f,1.0f,1.0f} }, //17 左 左下
	{ {  HALF,-HALF,  HALF},{1.0f,1.0f,1.0f} }, //18 左 右下
	{ {  HALF, HALF,  HALF},{1.0f,0.0f,1.0f} }, //19 左 右上

	{ { -HALF, HALF, -HALF},{0.0f,0.0f,3.0f} }, //20 右 左上
	{ { -HALF,-HALF, -HALF},{0.0f,1.0f,3.0f} }, //21 右 左下
	{ { -HALF,-HALF,  HALF},{1.0f,1.0f,3.0f} }, //22 右 右下
	{ { -HALF, HALF,  HALF},{1.0f,0.0f,3.0f} }, //23 右 右上
};

std::vector<uint16_t> Indices = {
		0,1,3,3,1,2,//前
		7,6,5,7,5,4,//后
		8,9,11,11,9,10,//top
		13,12,14,14,12,15,//bottom
		16,17,19,19,17,18,//left
		23,22,21,23,21,20
};

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}

private:
	void onInit() override
	{
		vertices = std::make_shared<std::vector<Vertex>>(Vertices);
		indices = std::make_shared<std::vector<uint16_t>>(Indices);
		
	}
	void initScene() override
	{
		SampleRender::initScene();
		auto cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -1.0f));

		main_scene.lock()->add_child(trans.lock());

		auto cube_obj = std::make_shared<vkd::Object>("Cube");
		auto cube_trans = cube_obj->add_comp<vkd::Transform>();
		cube_obj->add_comp<vkd::PipelineComp>("shader_23/skybox.vert","shader_23/skybox.frag");
		cube_obj->add_comp<vkd::DefRender>();
		cube_obj->add_comp<vkd::Mesh<Vertex, uint16_t>>(vertices, indices, "quad");
		cube_obj->add_comp<vkd::TextureArray>("skybox/skybox.json");

		main_scene.lock()->add_child(cube_trans.lock());
		
		
	}

	void onCleanUp() override
	{
		vkd::SampleRender::onCleanUp();
	}
private:
	std::shared_ptr<std::vector<Vertex>> vertices;
	std::shared_ptr<std::vector<uint16_t>> indices;
};

#include <event/event.hpp>
#include <json/reader.h>
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


