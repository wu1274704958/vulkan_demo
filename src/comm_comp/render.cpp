#include <comm_comp/camera.hpp>
#include  <comm_comp/render.hpp>
#include  <comm_comp/pipeline.hpp>

namespace vkd
{
	DefRender::DefRender(const DefRender& oth)
	{
		this->vp_buf = oth.vp_buf;
		this->vp = oth.vp;
	}

	void DefRender::awake()
	{
		vp_buf = gld::DefDataMgr::instance()->load_not_cache<gld::DataType::VkBuffer>(physical_dev(), device(), sizeof(Vp), vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

		ever_tick = true;
	}

	void DefRender::update_vp()
	{
		auto obj = object.lock();
		if (auto trans = obj->get_comp_raw<Transform>();trans)
		{
			if(auto scene = trans->get_scene().lock();scene)
			{
				if(auto cam = scene->get_bind_comp<Camera>();cam)
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
		auto f1 = update_descriptor();
		if(f1)update_vp();
		mesh = obj->get_comp_dyn<MeshInterface>();
		return f1 && !mesh.expired();
	}

	void DefRender::recreate_swapchain()
	{
		if(update_descriptor())
			update_vp();
	}

	bool DefRender::update_descriptor() const
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<PipelineComp>();
		if (pipeline)
		{
			const auto& descStes = pipeline->get_descriptorsets();
			vk::DescriptorBufferInfo buffInfo(vp_buf->buffer, 0, sizeof(Vp));
			vk::WriteDescriptorSet writeDescriptorSets = vk::WriteDescriptorSet(descStes[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, buffInfo);
			device().updateDescriptorSets(writeDescriptorSets, {});
			return true;
		}
		return false;
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

	std::shared_ptr<Component> DefRender::clone() const 
	{
		return std::make_shared<DefRender>(*this);
	}

	DefRenderInstance::DefRenderInstance(const DefRenderInstance&)
	{
	}


	bool DefRenderInstance::on_init()
	{
		if(!DefRender::on_init())
			return false;
		auto obj = object.lock();
		meshInstance = obj->get_comp_dyn<MeshInstanceInterface>();
		return !meshInstance.expired();
	}

	void DefRenderInstance::draw(vk::CommandBuffer& cmd)
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<PipelineComp>();
		auto trans = obj->get_comp_raw<Transform>();
		auto mesh_ptr = mesh.lock();
		auto mesh_ins = meshInstance.lock();

		if (trans && pipeline && mesh_ptr && mesh_ins)
		{
			const auto& mat = trans->get_matrix();
			cmd.pushConstants(pipeline->get_pipeline()->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), (void*)&mat);
			cmd.drawIndexed(mesh_ptr->index_count(), mesh_ins->instance_count(), 0, 0, 0);
		}
	}

	std::shared_ptr<Component> DefRenderInstance::clone() const
	{
		return std::make_shared<DefRenderInstance>(*this);
	}


	RenderNoIndex::RenderNoIndex(const RenderNoIndex&)
	{
	}

	void RenderNoIndex::draw(vk::CommandBuffer& cmd)
	{
		auto obj = object.lock();
		auto pipeline = obj->get_comp_raw<PipelineComp>();
		auto trans = obj->get_comp_raw<Transform>();
		auto mesh_ptr = mesh.lock();

		if (trans && pipeline && mesh_ptr)
		{
			const auto& mat = trans->get_matrix();
			cmd.pushConstants(pipeline->get_pipeline()->pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), (void*)&mat);
			cmd.draw(mesh_ptr->index_count(),1,0,0);
		}
	}

	std::shared_ptr<Component> RenderNoIndex::clone() const
	{
		return std::make_shared<RenderNoIndex>(*this);
	}

}

