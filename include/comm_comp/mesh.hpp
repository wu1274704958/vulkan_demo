#include <core/component.hpp>
#include <vulkan/vulkan.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_vk_res.hpp>
namespace vkd
{

	struct MeshInterface : public Component
	{
		virtual size_t index_count() const = 0;
	};

	struct MeshInstanceInterface : public Component
	{
		virtual size_t instance_count() const = 0;
	};

	template<typename VT,typename IT>
	struct Mesh : public MeshInterface
	{
		Mesh(std::shared_ptr<std::vector<VT>> vertices, std::shared_ptr <std::vector<IT>> indices) : vertices(vertices),indices(indices)
		{}
		void awake() override
		{
			if(vertices->empty() || indices->empty()) return;
			vertexBuf = gld::DefDataMgr::instance()->load_not_cache<gld::DataType::VkBuffer>(physical_dev(), device(), sizeof(VT) * vertices->size(),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
			indexBuf = gld::DefDataMgr::instance()->load_not_cache<gld::DataType::VkBuffer>(physical_dev(), device(), sizeof(IT) * indices->size(),
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);

			vertexBuf->copyToEx(physical_dev(), command_pool(), graphics_queue(), *vertices);
			indexBuf->copyToEx(physical_dev(), command_pool(), graphics_queue(), *indices);
		}
		bool on_init() override{return true;}
		void draw(vk::CommandBuffer& cmd) override
		{
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(0, vertexBuf->buffer, offset);
			cmd.bindIndexBuffer(indexBuf->buffer, 0, *IndexType);
		}
		int64_t idx() override {return  50;}
		void on_clean_up()
		{
			indexBuf.reset();
			vertexBuf.reset();
			vertices.reset();
			indices.reset();
		}
		size_t index_count() const override
		{
			return indices->size();
		}

		static constexpr std::optional<vk::IndexType> IndexType = wws::map_enum<IT, wws::ValList<vk::IndexType,
			vk::IndexType::eUint16,
			vk::IndexType::eUint32,
			vk::IndexType::eUint8EXT>,
			std::tuple<uint16_t,uint32_t,uint8_t>>();
	protected:
		std::shared_ptr<std::vector<VT>> vertices;
		std::shared_ptr<std::vector<IT>> indices;
		std::shared_ptr<gld::vkd::VkdBuffer> vertexBuf;
		std::shared_ptr<gld::vkd::VkdBuffer> indexBuf;
	};

	template<typename I>
	struct MeshInstance : public MeshInstanceInterface
	{
		MeshInstance(std::shared_ptr<std::vector<I>> instanceData,uint32_t binding = 1) : instanceData(instanceData), binding(binding)
		{}
		void awake() override
		{
			if (instanceData->empty()) return;
			instanceBuf = gld::DefDataMgr::instance()->load_not_cache<gld::DataType::VkBuffer>(physical_dev(), device(), sizeof(I) * instanceData->size(),
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
			
			instanceBuf->copyToEx(physical_dev(), command_pool(), graphics_queue(), *instanceData);
		}
		bool on_init() override { return true; }
		int64_t idx() override { return  60; }
		void draw(vk::CommandBuffer& cmd) override
		{
			vk::DeviceSize offset = 0;
			cmd.bindVertexBuffers(binding, instanceBuf->buffer, offset);
		}
		void on_clean_up()
		{
			instanceData.reset();
			instanceBuf.reset();
		}
		size_t instance_count() const override
		{
			return instanceData->size();
		}
		
	protected:
		uint32_t binding;
		std::shared_ptr<std::vector<I>> instanceData;
		std::shared_ptr<gld::vkd::VkdBuffer> instanceBuf;
	};
}
