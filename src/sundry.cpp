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
}