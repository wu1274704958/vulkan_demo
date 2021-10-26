#include <Sample/render.hpp>
#include <json.hpp>
#include <comm.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name)
	{}
private:
	void onInit() override {
		 pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device,"shader_23/quad.vert","shader_23/quad.frag",std::unordered_set<uint32_t>{},
			surfaceExtent,renderPass);
	}
	void onReCreateSwapChain() override {

	}
	void onRealDraw(vk::CommandBuffer& cmd) override {
		
	}
	void onCleanUp() override {
		pipeline.reset();
	}
	void onCleanUpPipeline() override {
 
	}

	std::shared_ptr<gld::vkd::PipelineData> pipeline;
};


void main()
{
	gld::DefResMgr::create_instance("../../../res");

	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	gld::DefDataMgr::instance()->clear_all();
	quad->cleanUp();
	delete quad;
}


