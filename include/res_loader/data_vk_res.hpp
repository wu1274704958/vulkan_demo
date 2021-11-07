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

		bool copyTo(void* data,vk::DeviceSize size,vk::DeviceSize offset = 0);
		bool copyToEx(vk::PhysicalDevice phy,vk::CommandPool cmdPool,vk::Queue queue,void* data, vk::DeviceSize size,vk::DeviceSize offset = 0);
		template<typename T>
		bool copyTo(const std::vector<T>& data, vk::DeviceSize offset = 0)
		{
			return copyTo((void*)data.data(), sizeof(T) * data.size(),offset);
		}
		template<typename T>
		bool copyTo( const T& data, vk::DeviceSize offset = 0)
		{
			return copyTo((void*)&data, sizeof(T),offset);
		}
		template<typename T>
		bool copyToEx(vk::PhysicalDevice phy, vk::CommandPool cmdPool, vk::Queue queue, const std::vector<T>& data, vk::DeviceSize offset = 0)
		{
			return copyToEx(phy,cmdPool,queue,(void*)data.data(),sizeof(T) * data.size(),offset);
		}
		template<typename T>
		bool copyToEx(vk::PhysicalDevice phy, vk::CommandPool cmdPool, vk::Queue queue, const T& data, vk::DeviceSize offset = 0)
		{
			return copyToEx(phy, cmdPool, queue, (void*)&data, sizeof(T),offset);
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

	struct VkdImage
	{
		vk::Device device;
		vk::Image image;
		vk::DeviceMemory mem;
		vk::Format format;
		vk::ImageTiling tiling;
	};

	template<typename ...Args>
	struct LoadVkImage {
		using RetTy = std::shared_ptr<VkdImage>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
		static std::string key_from_args(const std::string&, vk::PhysicalDevice, vk::Device, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
		static RealRetTy load(Args...);
	};

	using LoadVkImageTy = LoadVkImage<std::string, vk::PhysicalDevice, vk::Device, vk::BufferUsageFlags, vk::MemoryPropertyFlags>;
}