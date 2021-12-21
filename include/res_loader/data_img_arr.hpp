#pragma once

#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>
#include <data_vk_res.hpp>

namespace gld::vkd {

	template<typename ...Args>
	struct LoadVkImageArray {
		using RetTy = std::shared_ptr<VkdImage>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool, RetTy>;
		static std::string key_from_args(const std::string&, int);
		static std::string key_from_args(const std::string&, int, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)> onCreateImage = {},
			std::function<void(vk::SamplerCreateInfo&)> onCreateSample = {});
		static RealRetTy load(const std::string&, int, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)> onCreateImage = {},
			std::function<void(vk::SamplerCreateInfo&)> onCreateSample = {});
	};

}