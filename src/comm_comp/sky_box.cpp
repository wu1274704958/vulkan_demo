#include <comm_comp/sky_box.hpp>
#include <comm_comp/mesh.hpp>
#include <comm_comp/pipeline.hpp>
#include <comm_comp/render.hpp>
#include <misc_comp/MiscComp.hpp>

namespace vkd
{

	struct Vertex {
		glm::vec3 pos;
	};

#define HALF 1.0f

	std::vector<Vertex> Vertices = {
		{ {  HALF, HALF, HALF}	},//,{1.0f,1.0f,4.0f} }, //0 前左上
		{ {  HALF,-HALF, HALF}	},//,{1.0f,0.0f,4.0f} }, //1 前左下
		{ { -HALF,-HALF, HALF}	},//,{0.0f,0.0f,4.0f} }, //2 前右下
		{ { -HALF, HALF, HALF}	},//,{0.0f,1.0f,4.0f} }, //3 前右上
								
		{ {  HALF, HALF, -HALF}	},//,{0.0f,1.0f,2.0f} }, //4 后左上
		{ {  HALF,-HALF, -HALF}	},//,{0.0f,0.0f,2.0f} }, //5 后左下
		{ { -HALF,-HALF, -HALF}	},//,{1.0f,0.0f,2.0f} }, //6 后右下
		{ { -HALF, HALF, -HALF}	},//,{1.0f,1.0f,2.0f} }, //7 后右上

		{ {  HALF, HALF, -HALF}	},//,{0.0f,0.0f,5.0f} }, //8 上左上
		{ {  HALF, HALF,  HALF}	},//,{0.0f,1.0f,5.0f} }, //9 上左下
		{ { -HALF, HALF,  HALF}	},//,{1.0f,1.0f,5.0f} }, //10 上右下
		{ { -HALF, HALF, -HALF}	},//,{1.0f,0.0f,5.0f} }, //11 上右上
								
		{ {  HALF,-HALF, -HALF}	},//,{0.0f,1.0f,0.0f} }, //12 下左上
		{ {  HALF,-HALF,  HALF}	},//,{0.0f,0.0f,0.0f} }, //13 下左下
		{ { -HALF,-HALF,  HALF}	},//,{1.0f,0.0f,0.0f} }, //14 下右下
		{ { -HALF,-HALF, -HALF}	},//,{1.0f,1.0f,0.0f} }, //15 下右上
								
		{ {  HALF, HALF, -HALF}	},//,{1.0f,1.0f,1.0f} }, //16 左 左上
		{ {  HALF,-HALF, -HALF}	},//,{1.0f,0.0f,1.0f} }, //17 左 左下
		{ {  HALF,-HALF,  HALF}	},//,{0.0f,0.0f,1.0f} }, //18 左 右下
		{ {  HALF, HALF,  HALF}	},//,{.0f,1.0f,1.0f} }, //19 左 右上

		{ { -HALF, HALF, -HALF} },//,{0.0f,1.0f,3.0f} }, //20 右 左上
		{ { -HALF,-HALF, -HALF} },//,{0.0f,0.0f,3.0f} }, //21 右 左下
		{ { -HALF,-HALF,  HALF} },//,{1.0f,0.0f,3.0f} }, //22 右 右下
		{ { -HALF, HALF,  HALF} }//,{1.0f,1.0f,3.0f} }, //23 右 右上
	};
#undef HALF
	std::vector<uint16_t> Indices = {
			0,1,3,3,1,2,//前
			7,6,5,7,5,4,//后
			8,9,11,11,9,10,//top
			13,12,14,14,12,15,//bottom
			16,17,19,19,17,18,//left
			23,22,21,23,21,20
	};

	void SkyBox::onCreatePipeline(vk::GraphicsPipelineCreateInfo& info)
	{
		const_cast<vk::PipelineRasterizationStateCreateInfo*>(info.pRasterizationState)->cullMode = vk::CullModeFlagBits::eFront;
	}

	void SkyBox::awake()
	{
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device(), renderpass(), surface_extent(), "shader_23/skybox.vert", "shader_23/skybox.frag", 1, std::unordered_set<uint32_t>{},
			std::vector<uint32_t>{}, onCreatePipeline);
		not_draw = true;
		auto obj = object.lock();
		obj->add_comp<vkd::Mesh<Vertex, uint16_t>>(vertices, indices, "Sys_Skybox");
		obj->add_comp<vkd::PipelineComp>("shader_23/skybox.vert", "shader_23/skybox.frag");
		obj->add_comp<vkd::DefRender>();
		obj->add_comp<vkd::TextureCube>(path);
	}

	SkyBox::SkyBox(std::string path) : path(path)
	{
		
	}

	std::shared_ptr<Component> SkyBox::clone() const
	{
		return std::make_shared<SkyBox>(*this);
	}

	void SkyBox::recreate_swapchain()
	{
		gld::DefDataMgr::instance()->load<gld::DataType::PipelineSimple>(device(), renderpass(), surface_extent(), "shader_23/skybox.vert", "shader_23/skybox.frag", 1, std::unordered_set<uint32_t>{},
			std::vector<uint32_t>{}, onCreatePipeline);
	}

	void SkyBox::attach_scene(const std::weak_ptr<Scene>& scene)
	{
		
	}



	std::shared_ptr<std::vector<Vertex>> SkyBox::vertices = std::make_shared<std::vector<Vertex>>(Vertices);
	std::shared_ptr<std::vector<uint16_t>> SkyBox::indices = std::make_shared<std::vector<uint16_t>>(Indices);



	
}
