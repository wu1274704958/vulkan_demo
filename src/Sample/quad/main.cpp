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
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name){}

	bool dispatchEvent(const vkd::evt::Event& e) override
	{
		if (vkd::SampleRender::dispatchEvent(e)) return true;

		switch (e.type)
		{
		case vkd::evt::EventType::MouseDown:
		{
			auto& ev = e.GetEvent<vkd::evt::MouseButtonEvent>();
			mouseLastPos.x = (float)ev.x;
			mouseLastPos.y = (float)ev.y;
		}
			break;
		case vkd::evt::EventType::MouseMove:
			if (eventConstructor.isMouseBtnPress(vkd::evt::MouseBtnLeft))
			{
				auto& ev = e.GetEvent<vkd::evt::MouseButtonEvent>();
				mouseMoveOffset.x = ((float)ev.x - mouseLastPos.x) / surfaceExtent.width;
				mouseMoveOffset.y =  ((float)ev.y - mouseLastPos.y) / surfaceExtent.height;
				mouseLastPos.x = (float)ev.x;
				mouseLastPos.y = (float)ev.y;
				return true;
			}
		break;
		case vkd::evt::EventType::MouseUp:
			mouseMoveOffset.y = mouseMoveOffset.x = 0.0f;
			break;
		default:
			break;
		}
		return false;
	}
private:
	void onInit() override {

		uniformObj.proj = glm::perspective(glm::radians(45.0f), (float)surfaceExtent.width / (float)surfaceExtent.height, 0.1f, 100.0f);
		uniformObj.view = glm::translate(glm::mat4(1.0f),glm::vec3(0.f,0.f,-4.0f));

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

		image = dataMgr->load<gld::DataType::VkImage>("textures/texture.jpg", STBI_rgb_alpha, physicalDevice, device, commandPool, graphicsQueue, nullptr, [](vk::SamplerCreateInfo& info) {
			info.minFilter = vk::Filter::eLinear;
		});

		onReCreateSwapChain();
		

	}
	void onReCreateSwapChain() override {
		pipeline = gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent, "shader_23/quad.vert", "shader_23/quad.frag", 1);
		descSets = pipeline->allocDescriptorSets();

		vk::DescriptorBufferInfo buffInfo(uniformBuf->buffer, 0, sizeof(UniformBufferObject));
		vk::DescriptorImageInfo imageInfo(image->sample,image->view,vk::ImageLayout::eShaderReadOnlyOptimal);
		std::array<vk::WriteDescriptorSet,2> writeDescriptorSets = {
			vk::WriteDescriptorSet(descSets[0], 0, 0, vk::DescriptorType::eUniformBuffer,{}, buffInfo),
			vk::WriteDescriptorSet(descSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo, {})
		};
		device.updateDescriptorSets(writeDescriptorSets, {});

		uniformObj.proj = glm::perspective(glm::radians(45.0f), (float)surfaceExtent.width / (float)surfaceExtent.height, 0.1f, 100.0f);
	}
	void onRealDraw(vk::CommandBuffer& cmd) override {
		cmd.beginRenderPass(renderPassBeginInfo,vk::SubpassContents::eInline);
		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);
		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipelineLayout, 0, descSets, {});
		vk::Viewport viewport(0, 0, (float)surfaceExtent.width, (float)surfaceExtent.height, 0.0f, 1.0f);
		cmd.setViewport(0, viewport);
		vk::Rect2D scissor({}, surfaceExtent);
		cmd.setScissor(0, scissor);
		vk::DeviceSize offset = 0;
		cmd.bindVertexBuffers(0, verticesBuf->buffer, offset);
		cmd.bindIndexBuffer(indicesBuf->buffer, 0, vk::IndexType::eUint16);
		drawQuad(cmd, glm::vec3(0.7f, 0.0f, 1.0f));
		drawQuad(cmd, glm::vec3(0.0f, 0.0f, 0.0f));
		cmd.endRenderPass();
	}
	void drawQuad(vk::CommandBuffer& cmd,glm::vec3 pos)
	{
		auto model = set_model(m_rotate, pos);
		cmd.pushConstants(pipeline->pipelineLayout,vk::ShaderStageFlagBits::eVertex,0,sizeof(glm::mat4),(void*)&model);
		cmd.drawIndexed(indices.size(), 1, 0, 0, 0);
	}
	void onCleanUp() override {
		indicesBuf.reset();
		verticesBuf.reset();
		uniformBuf.reset();
		image.reset();
	}
	void onCleanUpPipeline() override {
		device.freeDescriptorSets(pipeline->descriptorPool,descSets);
		pipeline.reset();
		gld::DefDataMgr::instance()->rm_cache<gld::DataType::PipelineSimple>("shader_23/quad.vert", "shader_23/quad.frag");
	}
	void onUpdate(float delta) override
	{
		m_rotate += 26.f * delta;
		worldRotate += mouseMoveOffset * delta * 42000.0f;
		mouseMoveOffset = glm::vec2(0.0f);
		uniformObj.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, -4.0f));
		uniformObj.view = glm::rotate(uniformObj.view, glm::radians(worldRotate.x), glm::vec3(0.0f, 1.0f, 0.0f));
		uniformObj.view = glm::rotate(uniformObj.view, glm::radians(-worldRotate.y), glm::vec3(1.0f, 0.0f, 0.0f));
		uniformBuf->copyTo(uniformObj);
	}

	glm::mat4 set_model(float rotate,glm::vec3 pos)
	{
		auto model = glm::translate(glm::mat4(1.0f), pos);
		model = glm::rotate(model, glm::radians(27.0f), glm::vec3(0.0f, 1.f, 0.f));
		model = glm::rotate(model, glm::radians(m_rotate), glm::vec3(0.f, 0.f, 1.0f));
		return model;
	}

	std::shared_ptr<gld::vkd::PipelineData> pipeline;
	std::vector<vk::DescriptorSet> descSets;
	std::shared_ptr<gld::vkd::VkdBuffer> verticesBuf,indicesBuf,uniformBuf;
	std::shared_ptr<gld::vkd::VkdImage> image;
	
	UniformBufferObject uniformObj;
	float m_rotate = 0.0f;
	glm::vec2 mouseMoveOffset = glm::vec2(0.0f, 0.0f);
	glm::vec2 mouseLastPos;
	glm::vec2 worldRotate = glm::vec2(0.0f,0.0f);
};

#include <event/event.hpp>
int main()
{
	gld::DefResMgr::create_instance(std::make_tuple("../../../res"));
	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	gld::DefDataMgr::instance()->clear_all();
	quad->cleanUp();
	delete quad;
	return 0;
}


