#include <Sample/render.hpp>
#include <json.hpp>
#include <comm.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>


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

	try{
		auto spv = gld::DefResMgr::instance()->load<gld::ResType::spirv_with_meta>("shader_23/quad.vert");
	}
	catch (std::runtime_error e)
	{
		printf("%s\n",e.what());
	}

	PREPARE_CT_STR("abc");
	PREPARE_CT_STR(L"abc");

	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	quad->cleanUp();
	delete quad;
}


