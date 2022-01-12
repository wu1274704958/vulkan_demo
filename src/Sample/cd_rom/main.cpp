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
#include <assimp/scene.h>
#include <comm_comp/sky_box.hpp>
#include <comm_comp/pipeline.hpp>
#include <comm_comp/mesh.hpp>
#include <comm_comp/render.hpp>
#include <misc_comp/MiscComp.hpp>
#include "shape.hpp"
struct Vertex
{
	glm::vec2 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}

private:
	void onInit() override
	{
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device,renderPass,surfaceExtent, "shader_23/cd_rom.vert", "shader_23/cd_rom.frag",1,
			std::unordered_set<uint32_t>{},std::vector<uint32_t>{},OnCreatePipeline);
	}
	void initScene() override
	{
		SampleRender::initScene();
		auto cam_obj = std::make_shared<vkd::Object>("Camera");
		auto trans = cam_obj->add_comp<vkd::Transform>();
		auto cam = cam_obj->add_comp<vkd::Showcase>();
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -2.0f));

		main_scene.lock()->add_child(trans.lock());
		//构造天空盒并加到场景上
		auto cube_obj = std::make_shared<vkd::Object>("Skybox");
		auto cube_trans = cube_obj->add_comp<vkd::Transform>();
		cube_obj->add_comp<vkd::SkyBox>("skybox/skybox.json");
		main_scene.lock()->add_child(cube_trans.lock());
		
		std::shared_ptr<std::vector<Vertex>> vertices = std::make_shared<std::vector<Vertex>>();

		shape::Circle circle;
		auto vcu = circle.generate_vcu();

		for(;;)
		{
			auto v = vcu->next();
			if(!v) break;
			vertices->push_back(Vertex{
				.pos = std::get<0>(*v),
				.normal = std::get<1>(*v),
				.uv = std::get<2>(*v)
			});
		}

		auto cd = std::make_shared<vkd::Object>();
		auto cd_trans = cd->add_comp<vkd::Transform>();

		cd_trans.lock()->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
		cd_trans.lock()->set_position(glm::vec3(0.0f, 0.0f, 0.0f));

		cd->add_comp<vkd::PipelineComp>("shader_23/cd_rom.vert", "shader_23/cd_rom.frag");
		cd->add_comp<vkd::MeshNoIndex<Vertex>>(vertices, "Cd");
		//在“心”上添加 天空盒采样器 组件
		cd->add_comp<vkd::SkyBoxSampler>(1, 0);//binding,描述符集合index
		cd->add_comp<vkd::RenderNoIndex>();

		main_scene.lock()->add_child(cd_trans.lock());
	}

	void onReCreateSwapChain() override
	{
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent, "shader_23/cd_rom.vert", "shader_23/cd_rom.frag", 1,
			std::unordered_set<uint32_t>{}, std::vector<uint32_t>{}, OnCreatePipeline);
		SampleRender::onReCreateSwapChain();
	}

	static void OnCreatePipeline(vk::GraphicsPipelineCreateInfo& info)
	{
		const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState)->topology = vk::PrimitiveTopology::eTriangleFan;
		const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState)->cullMode = vk::CullModeFlagBits::eFront;
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

