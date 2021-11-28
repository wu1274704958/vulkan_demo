#include <comm_comp/camera.hpp>
#include  <comm_comp/render.hpp>
#include  <comm_comp/pipeline.hpp>

namespace vkd
{

	void DefRender::awake()
	{
		vp_buf = gld::DefDataMgr::instance()->load_not_cache<gld::DataType::VkBuffer>(physical_dev(), device(), sizeof(Vp), vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	}

	void DefRender::update_vp()
	{
		auto obj = object.lock();
		if (auto trans = obj->get_comp_raw<Transform>();trans)
		{
			if(auto scene = trans->get_scene().lock();scene)
			{
				if(auto cam = scene->get_camera();cam)
				{
					if(cam->is_dirty())
					{
						vp.view = cam->get_matrix_v();
						vp.proj = cam->get_matrix_p();
						vp_buf->copyTo(vp);
					}
				}
			}
		}
	}


	bool DefRender::on_init()
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<PipelineComp>();
		if (pipeline)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorBufferInfo buffInfo(vp_buf->buffer, 0, sizeof(Vp));
			vk::WriteDescriptorSet writeDescriptorSets = vk::WriteDescriptorSet(descStes[0], 0, 0, vk::DescriptorType::eUniformBuffer,{}, buffInfo);
			device().updateDescriptorSets(writeDescriptorSets, {});
		}else return false;
		update_vp();
		mesh = obj->get_comp_dyn<MeshComp>();
		return mesh.expired();
	}

	void DefRender::late_update(float delta)
	{
		update_vp();	
	}

	void DefRender::draw(vk::CommandBuffer& cmd)
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<PipelineComp>();
		auto trans = obj->get_comp_raw<Transform>();
		auto mesh_ptr = mesh.lock();

		if(trans && pipeline && mesh_ptr)
		{
			const auto& mat = trans->get_matrix();
			cmd.pushConstants(pipeline->get_pipeline()->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), (void*)&mat);
			cmd.drawIndexed(mesh_ptr->index_count(),1,0,0,0);
		}
		
	}

	void DefRender::on_clean_up()
	{
		vp_buf.reset();
	}


}

