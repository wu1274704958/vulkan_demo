#include <res_loader/data_vk_res.hpp>
#include <sundry.hpp>
namespace gld::vkd {

	uint32_t findMemoryType(vk::PhysicalDevice phyDev, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties = phyDev.getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
			{
				return i;
			}
		}
		return std::numeric_limits<uint32_t>::max();
	}

	bool createBuffer(vk::PhysicalDevice phyDev, vk::Device device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memProperty, vk::Buffer& buffer, vk::DeviceMemory& mem)
	{
		vk::BufferCreateInfo buffer_info = {};
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = vk::SharingMode::eExclusive;

		buffer = device.createBuffer(buffer_info);
		if (!buffer) return false;
		auto memRequirments = device.getBufferMemoryRequirements(buffer);

		vk::MemoryAllocateInfo alloc_info = {};
		alloc_info.allocationSize = memRequirments.size;
		alloc_info.memoryTypeIndex = findMemoryType(phyDev, memRequirments.memoryTypeBits, memProperty);

		mem = device.allocateMemory(alloc_info);
		if (!buffer) return false;
		device.bindBufferMemory(buffer, mem, 0);
			
		return true;
	}

	std::string CreateVkBufferTy::key_from_args(const std::string& name, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags prop)
	{
		return sundry::format_tup('#',name,(uint32_t)usage,(uint32_t)prop);
	}

	std::string CreateVkBufferTy::key_from_args(const std::string& name, vk::PhysicalDevice, vk::Device, vk::DeviceSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags prop)
	{
		return sundry::format_tup('#', name, (uint32_t)usage, (uint32_t)prop);
	}

	CreateVkBufferTy::RealRetTy CreateVkBufferTy::load(const std::string&, vk::PhysicalDevice phy, vk::Device dev, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags prop)
	{
		auto buff = std::make_shared<VkdBuffer>();
		if (createBuffer(phy, dev, size, usage, prop, buff->buffer, buff->mem))
		{
			buff->device = dev;
			buff->usage = usage;
			buff->memProperty = prop;
			return std::make_tuple(true,buff);
		}
		return std::make_tuple(false,nullptr);
	}

	bool VkdBuffer::copyTo(void* data, vk::DeviceSize size, vk::DeviceSize offset)
	{
		if ((memProperty & vk::MemoryPropertyFlagBits::eHostVisible) == vk::MemoryPropertyFlagBits::eHostVisible)
		{
			void* dst = device.mapMemory(mem, offset, size);
			memcpy(dst, data, size);
			device.unmapMemory(mem);
			return true;
		}
		return false;
	}
	bool VkdBuffer::copyToEx(vk::PhysicalDevice phy, vk::CommandPool cmdPool, vk::Queue queue, void* data, vk::DeviceSize size, vk::DeviceSize offset)
	{
		VkdBuffer buf;
		if (createBuffer(phy, device, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buf.buffer, buf.mem))
		{
			buf.usage = vk::BufferUsageFlagBits::eTransferSrc;
			buf.memProperty = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			buf.device = device;
			buf.copyTo(data,size,offset);

			auto cmd = sundry::beginSingleTimeCommands(device,cmdPool);
			vk::BufferCopy region({},{},size);
			cmd.copyBuffer(buf.buffer,this->buffer, region);
			sundry::endSingleTimeCommands(device,cmd,cmdPool,queue);
			return true;
		}
		return false;
	}

	
}