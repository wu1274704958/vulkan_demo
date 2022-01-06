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

struct Vertex
{
	glm::vec3 pos,normal;
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
		trans.lock()->set_position(glm::vec3(0.f, 0.f, -1.0f));

		main_scene.lock()->add_child(trans.lock());

		auto cube_obj = std::make_shared<vkd::Object>("Skybox");
		auto cube_trans = cube_obj->add_comp<vkd::Transform>();
		cube_obj->add_comp<vkd::SkyBox>("skybox/skybox.json");

		main_scene.lock()->add_child(cube_trans.lock());

		auto [vertices,indices] = load_model();
	}

	std::tuple<std::shared_ptr<std::vector<Vertex>>,std::shared_ptr<std::vector<uint16_t>>>
	load_model()
	{
		std::shared_ptr<Assimp::Importer> heart = gld::DefResMgr::instance()->load<gld::ResType::model>("heart_lp/heart_lp.obj");

		aiNode* node = heart->GetScene()->mRootNode;
		while(node && node->mNumMeshes <= 0 && node->mNumChildren > 0)
		{
			node = node->mChildren[0];
		}
		if(!node || node->mNumMeshes <= 0)
			return std::make_tuple(nullptr,nullptr);
		aiMesh* mesh = heart->GetScene()->mMeshes[ node->mMeshes[0] ];

		auto vertices = std::make_shared<std::vector<Vertex>>();

		for(int i = 0;i < mesh->mNumVertices;++i)
		{
			auto v = mesh->mVertices[i];
			auto n = mesh->mNormals[i];
			vertices->push_back({ 
				.pos = glm::vec3(v.x,v.y,v.z),
				.normal = glm::vec3(n.x,n.y,n.z)
			});
		}

		auto indices = std::make_shared<std::vector<uint16_t>>();
		for(int i = 0;i < mesh->mNumFaces;++i)
		{
			aiFace face = mesh->mFaces[i];
			for(int j = 0;j < face.mNumIndices;++j)
				indices->push_back(static_cast<uint16_t>(face.mIndices[j]));
		}

		return std::make_tuple(vertices,indices);
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


