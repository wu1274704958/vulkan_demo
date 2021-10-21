#include <Sample/render.hpp>
#include <json.hpp>
#include <comm.hpp>
#include <res_loader/resource_mgr.hpp>


class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name)
	{}
private:
	void onInit() override {

	}
	void onReCreateSwapChain() override {

	}
	void onRealDraw(vk::CommandBuffer& cmd) override {

	}
	void onCleanUp() override {

	}
	void onCleanUpPipeline() override {

	}
};


void main()
{
	gld::DefResMgr::create_instance("../../../res");
	gld::ResMgrWithGlslPreProcess::create_instance("../../../res");

	try{
		auto glsl = gld::ResMgrWithGlslPreProcess::instance()->load<gld::ResType::text>("shader_23/quad.vert");
		printf("%s\n", glsl->c_str());
	}
	catch (std::runtime_error e)
	{
		printf("%s\n",e.what());
	}


	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	quad->cleanUp();
	delete quad;
}


