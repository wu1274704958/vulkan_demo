#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

namespace gld::vkd {

	struct VkdBuffer
	{
		vk::Device device;
		vk::Buffer buffer;
		vk::DeviceMemory mem;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags memProperty;

		bool copyTo(void* data,vk::DeviceSize size);
		bool copyToEx(vk::PhysicalDevice phy,vk::CommandPool cmdPool,vk::Queue queue,void* data, vk::DeviceSize size);
		template<typename T>
		bool copyTo(const std::vector<T>& data)
		{
			return copyTo((void*)data.data(), sizeof(T) * data.size());
		}
		template<typename T>
		bool copyTo( const T& data)
		{
			return copyTo((void*)&data, sizeof(T));
		}
		template<typename T>
		bool copyToEx(vk::PhysicalDevice phy, vk::CommandPool cmdPool, vk::Queue queue, const std::vector<T>& data)
		{
			return copyToEx(phy,cmdPool,queue,(void*)data.data(),sizeof(T) * data.size());
		}
		template<typename T>
		bool copyToEx(vk::PhysicalDevice phy, vk::CommandPool cmdPool, vk::Queue queue, const T& data)
		{
			return copyToEx(phy, cmdPool, queue, (void*)&data, sizeof(T));
		}
		~VkdBuffer() {
			if (!device) return;
			if (mem) device.freeMemory(mem);
			if (buffer) device.destroyBuffer(buffer);
		}
	};
	template<typename ...Args>
	struct CreateVkBuffer {
		using RetTy = std::shared_ptr<VkdBuffer>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
		static std::string key_from_args(const std::string&,vk::PhysicalDevice,vk::Device,vk::DeviceSize,vk::BufferUsageFlags ,vk::MemoryPropertyFlags);
		static RealRetTy load(const std::string&, vk::PhysicalDevice, vk::Device, vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
	};

	using CreateVkBufferTy = CreateVkBuffer<std::string, vk::PhysicalDevice, vk::Device, vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags>;
}