#include <core/object.hpp>

#include <comm_comp/transform.hpp>
#include <comm_comp/pipeline.hpp>
#include <sample/render.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>
#include <json.hpp>
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

#include <event/event.hpp>

#include <sample/shape.hpp>
#include <ranges>

class CD_ROM : public vkd::SampleRender {
public:
	CD_ROM(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers, sample_name) {}
	using Vertex = std::tuple<glm::vec2, glm::vec3, glm::vec2>;
private:
	std::shared_ptr<std::vector<Vertex>> vertices;
	std::shared_ptr<std::vector<uint16_t>> indices;
	
	void onCreatePipeline(vk::GraphicsPipelineCreateInfo& info)
	{
		const_cast<vk::PipelineInputAssemblyStateCreateInfo*>(info.pInputAssemblyState)->topology = vk::PrimitiveTopology::eTriangleFan;
	}

	void onInit() override
	{
		shape::Circle circle(180);
		auto vertex = circle.generate_vcu()->vector();


		vertices = std::make_shared<std::vector<Vertex>>( vertex );

		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device, renderPass, surfaceExtent,
			"shader_23/cd_rom.vert", "shader_23/cd_rom.frag", onCreatePipeline);
	}
};

int main()
{
	
	return 0;
}

