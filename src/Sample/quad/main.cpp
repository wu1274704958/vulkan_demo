#include <Sample/render.hpp>
#include <json.hpp>
#include <comm.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	 0, 2, 1, 0, 3, 2
};

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name){}
private:
	void onInit() override {

		uniformObj.proj = glm::perspective(glm::radians(45.0f), (float)surfaceExtent.width / (float)surfaceExtent.height, 0.1f, 100.0f);
		uniformObj.view = glm::translate(glm::mat4(1.0f),glm::vec3(0.f,0.f,-4.0f));
		uniformObj.model = glm::rotate(glm::mat4(1.0f),glm::radians(7.0f),glm::vec3(0.0f,1.f,0.f));

		auto dataMgr = gld::DefDataMgr::instance();
		verticesBuf = dataMgr->load<gld::DataType::VkBuffer>("vertices",physicalDevice,device,sizeof(Vertex) * vertices.size(),
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,vk::MemoryPropertyFlagBits::eDeviceLocal);
		verticesBuf->copyToEx(physicalDevice,commandPool,graphicsQueue,vertices);

		indicesBuf = dataMgr->load<gld::DataType::VkBuffer>("indices", physicalDevice, device, sizeof(uint16_t) * indices.size(),
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
		indicesBuf->copyToEx(physicalDevice, commandPool, graphicsQueue, indices);

		uniformBuf = dataMgr->load<gld::DataType::VkBuffer>("uniform",physicalDevice,device,sizeof(UniformBufferObject),vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostCoherent|vk::MemoryPropertyFlagBits::eHostVisible);
		uniformBuf->copyTo(uniformObj);

		onReCreateSwapChain();
		

	}
	void onReCreateSwapChain() override {
		pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent, "shader_23/quad.vert", "shader_23/quad.frag");
		{
			vk::DescriptorSetAllocateInfo info(pipeline->descriptorPool, 1, &pipeline->setLayout);
			descSets = device.allocateDescriptorSets(info);
		}

		vk::DescriptorBufferInfo buffInfo(uniformBuf->buffer, 0, sizeof(UniformBufferObject));
		vk::WriteDescriptorSet writeDescriptorSet(descSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, buffInfo);

		device.updateDescriptorSets(writeDescriptorSet, {});
	}
	void onRealDraw(vk::CommandBuffer& cmd) override {
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics,pipeline->pipeline);
		vk::Viewport viewport(0,0,(float)surfaceExtent.width,(float)surfaceExtent.height,0.0f,1.0f);
		cmd.setViewport(0,viewport);
		vk::Rect2D scissor({},surfaceExtent);
		cmd.setScissor(0,scissor);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,pipeline->pipelineLayout,0,descSets,{});
		vk::DeviceSize offset = 0;
		cmd.bindVertexBuffers(0,verticesBuf->buffer,offset);
		cmd.bindIndexBuffer(indicesBuf->buffer,0,vk::IndexType::eUint16);
		cmd.drawIndexed(indices.size(),1,0,0,0);
	}
	void onCleanUp() override {
		indicesBuf.reset();
		verticesBuf.reset();
		uniformBuf.reset();
	}
	void onCleanUpPipeline() override {
		pipeline.reset();
		gld::DefDataMgr::instance()->rm_cache<gld::DataType::PipelineSimple>("shader_23/quad.vert", "shader_23/quad.frag");
	}
	void onUpdate(float delta) override
	{
		uniformObj.model = glm::rotate(glm::mat4(1.0f), glm::radians(27.0f), glm::vec3(0.0f, 1.f, 0.f));
		uniformObj.model = glm::rotate(uniformObj.model, glm::radians(m_rotate), glm::vec3(0.f, 0.f, 1.0f));
		uniformBuf->copyTo(uniformObj);
		m_rotate += 0.1f;
	}

	std::shared_ptr<gld::vkd::PipelineData> pipeline;
	std::vector<vk::DescriptorSet> descSets;
	std::shared_ptr<gld::vkd::VkdBuffer> verticesBuf,indicesBuf,uniformBuf;
	UniformBufferObject uniformObj;
	float m_rotate = 0.0f;
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


