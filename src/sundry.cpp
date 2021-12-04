#include <sundry.hpp>

namespace sundry
{
	vk::CommandBuffer beginSingleTimeCommands(vk::Device device, vk::CommandPool cmdPool)
	{
		vk::CommandBufferAllocateInfo cbAlloc_info = {};
		cbAlloc_info.level = vk::CommandBufferLevel::ePrimary;
		cbAlloc_info.commandPool = cmdPool;
		cbAlloc_info.commandBufferCount = 1;

		vk::CommandBuffer cb;
		if (device.allocateCommandBuffers(&cbAlloc_info, &cb) != vk::Result::eSuccess)
			return cb;
		vk::CommandBufferBeginInfo begin_info = {};
		begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cb.begin(begin_info);
		return cb;
	}
	void endSingleTimeCommands(vk::Device device, vk::CommandBuffer cb, vk::CommandPool cmdPool, vk::Queue queue,std::function<void()> onWait)
	{
		cb.end();
		vk::SubmitInfo submit_info = {};
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cb;
		vk::FenceCreateInfo info;
		auto fence = device.createFence(info);
		queue.submit(submit_info, fence);
		if(onWait) onWait();
		while (device.waitForFences(fence, true, std::numeric_limits<std::uint64_t>::max()) == vk::Result::eTimeout) {}
		device.freeCommandBuffers(cmdPool, cb);
		device.destroyFence(fence);
	}
	uint32_t findMemoryType(vk::PhysicalDevice phyDev, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
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
		alloc_info.memoryTypeIndex = sundry::findMemoryType(phyDev, memRequirments.memoryTypeBits, memProperty);

		mem = device.allocateMemory(alloc_info);
		if (!buffer) return false;
		device.bindBufferMemory(buffer, mem, 0);

		return true;
	}

	bool createImage(vk::PhysicalDevice phyDev, vk::Device dev, uint32_t w, uint32_t h, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlagBits memProp, vk::Image& img, vk::DeviceMemory& mem, std::function<void(vk::ImageCreateInfo&)> onCreateImage)
	{
		vk::ImageCreateInfo info;
		info.extent = vk::Extent3D(w, h, 1);
		info.format = format;
		info.tiling = tiling;
		info.imageType = vk::ImageType::e2D;
		info.usage = usage;
		info.arrayLayers = 1;
		info.mipLevels = 1;
		info.initialLayout = vk::ImageLayout::eUndefined;
		info.sharingMode = vk::SharingMode::eExclusive;
		info.samples = vk::SampleCountFlagBits::e1;
		if (onCreateImage) onCreateImage(info);
		img = dev.createImage(info);
		if (!img) return false;
		mem = sundry::allocVkMemory(dev, phyDev, memProp, img);
		if (!mem) {
			dev.destroyImage(img);
			return false;
		}
		return true;
	}
}