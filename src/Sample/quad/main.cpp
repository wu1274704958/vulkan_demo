#include <Sample/render.hpp>
#include <json.hpp>
#include <comm.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>
#include <stb_image.h>
#include <glm/glm.hpp>

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
};
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

std::vector<uint16_t> indices = {
	 0, 1, 2, 2, 3, 0
};

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name)
	{}
private:
	void onInit() override {
		onReCreateSwapChain();

		auto dataMgr = gld::DefDataMgr::instance();
		verticesBuf = dataMgr->load<gld::DataType::VkBuffer>("vertices",physicalDevice,device,sizeof(Vertex) * vertices.size(),
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,vk::MemoryPropertyFlagBits::eDeviceLocal);
		verticesBuf->copyToEx(physicalDevice,commandPool,graphicsQueue,vertices);

		indicesBuf = dataMgr->load<gld::DataType::VkBuffer>("indices", physicalDevice, device, sizeof(uint16_t) * indices.size(),
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
		indicesBuf->copyToEx(physicalDevice, commandPool, graphicsQueue, indices);
	}
	void onReCreateSwapChain() override {
		pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent, "shader_23/quad.vert", "shader_23/quad.frag");
		{
			vk::DescriptorSetAllocateInfo info(pipeline->descriptorPool, 1, &pipeline->setLayout);
			descSets = device.allocateDescriptorSets(info);
		}
	}
	void onRealDraw(vk::CommandBuffer& cmd) override {
		
	}
	void onCleanUp() override {
		indicesBuf.reset();
		verticesBuf.reset();
	}
	void onCleanUpPipeline() override {
		pipeline.reset();
		gld::DefDataMgr::instance()->rm_cache<gld::DataType::PipelineSimple>("shader_23/quad.vert", "shader_23/quad.frag");
	}

	std::shared_ptr<gld::vkd::PipelineData> pipeline;
	std::vector<vk::DescriptorSet> descSets;
	std::shared_ptr<gld::vkd::VkdBuffer> verticesBuf,indicesBuf;
};


void main()
{
	gld::DefResMgr::create_instance(std::make_tuple("../../../res"));
	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	gld::DefDataMgr::instance()->clear_all();
	quad->cleanUp();
	delete quad;
}


