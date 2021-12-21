#include <res_loader/data_img_arr.hpp>
#include <sundry.hpp>
#include <res_loader/resource_mgr.hpp>
#include <json/value.h>
#include <res_loader/data_comm.hpp>
#include <common.hpp>
#include <stb_image.h>

namespace gld::vkd
{
	using LoadVkImageArrayTy = LoadVkImageArray< std::string, int, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)>,
		std::function<void(vk::SamplerCreateInfo&)>>;

	template<>
	std::string LoadVkImageArrayTy::key_from_args(const std::string& key, int comp)
	{
		return sundry::format_tup('#',key,comp);
	}
	template<>
	std::string LoadVkImageArrayTy::key_from_args(const std::string& key, int comp, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)>,
		std::function<void(vk::SamplerCreateInfo&)>)
	{
		return sundry::format_tup('#', key, comp);
	}

	inline bool createTempBuf(vk::PhysicalDevice phyDev, vk::Device dev, vk::DeviceSize size, void* data, vk::Buffer& buf, vk::DeviceMemory& mem, vk::DeviceSize offset = 0)
	{
		if (!sundry::createBuffer(phyDev, dev, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buf, mem))
			return false;
		void* dst = dev.mapMemory(mem, offset, size);
		memcpy(dst, data, size);
		dev.unmapMemory(mem);
		return true;
	}

	
	template <>
	LoadVkImageArrayTy::RealRetTy LoadVkImageArrayTy::load(const std::string& key, int flag, vk::PhysicalDevice phyDev, vk::Device dev, vk::CommandPool cmdPool, vk::Queue queue,
		std::function<void(vk::ImageCreateInfo&)> onCreateImage, std::function<void(vk::SamplerCreateInfo&)> onCreateSample)
	{
		std::shared_ptr<Json::Value> json = ::gld::DefResMgr::instance()->load<gld::ResType::json>(key);
		if(!json) return std::make_tuple(false,nullptr);

		Json::Value& conf = *json;
		if(Json::Value& v = conf["type"];v.isNull() || v.asInt() != static_cast<int>(DataType::VkImageArray))
			return std::make_tuple(false, nullptr);
		bool absolute = false;
		if (Json::Value& v = conf["absolute"]; !v.isNull() || v.asBool())
			absolute = true;
		std::string_view parent = absolute ? "" : key;
		if(!absolute && !wws::up_path<'/'>(parent))
			return std::make_tuple(false, nullptr);
		Json::Value& images = conf["images"];
		if(images.isNull() || !images.isArray() || images.size() == 0)
			return std::make_tuple(false, nullptr);

		std::vector<vk::BufferImageCopy> bufferCopyRegions;
		std::vector<std::byte> imagesData;
		vk::DeviceSize bufOffset = 0,bufferSize = 0;
		auto imageSize = images.size();
		vk::BufferImageCopy copy;
		copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		copy.imageSubresource.layerCount = imageSize;
		copy.imageSubresource.mipLevel = 0;
		std::optional<vk::Format> format;
		for(int i = 0;i < imageSize;++i)
		{
			if(images[i].isNull() || !images[i].isString())continue;
			std::string path;
			if(absolute)
			{
				path = images.asString();
			}else
			{
				path = std::string(parent);
				path += '/';
				path += images.asString();
			}
			std::shared_ptr<StbImage> img = gld::DefResMgr::instance()->load<ResType::image>(path, flag);
			if (!img) return std::make_tuple(false, nullptr);
			if (flag == STBI_rgb_alpha) img->channel = 4;
			if(!format)
				format = wws::map_enum<wws::ValList<int, 1, 3, 4>,wws::ValList<vk::Format,vk::Format::eR8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm>>(img->channel);
			if (!format) return std::make_tuple(false, nullptr);
			
			copy.setImageExtent(vk::Extent3D(img->width,img->height,1));
			copy.imageSubresource.baseArrayLayer = i;
			copy.bufferOffset = bufOffset;
			bufferCopyRegions.push_back(copy);

			vk::DeviceSize size = img->width * img->height * img->channel;
			bufferSize += size;
			imagesData.resize(bufferSize);
			memcpy(imagesData.data() + bufOffset,img->data,size);
			bufOffset += size;
		}
		
		vk::Buffer tmpBuffer; vk::DeviceMemory tmpMem;
		if (!createTempBuf(phyDev, dev, imageSize, (void*)imagesData.data(), tmpBuffer, tmpMem))
			return std::make_tuple(false, nullptr);

		auto res = std::make_shared<VkdImage>();

		if (!sundry::createImage(phyDev, dev, bufferCopyRegions[0].imageExtent.width, bufferCopyRegions[0].imageExtent.height, *format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal,
			res->image, res->mem, onCreateImage))
			return std::make_tuple(false, nullptr);

		res->arrayLayers = imageSize;
		res->mipLevels = 1;
		res->device = dev;
		res->format = *format;
		res->tiling = vk::ImageTiling::eOptimal;

		vk::CommandBuffer cmd = sundry::beginSingleTimeCommands(dev, cmdPool);
		vk::ImageMemoryBarrier barrier;
		barrier.image = res->image;
		barrier.oldLayout = vk::ImageLayout::eUndefined;
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, imageSize);
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0, {}, {}, barrier);
		cmd.copyBufferToImage(tmpBuffer, res->image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegions);
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNoneKHR;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, (vk::DependencyFlagBits)0, {}, {}, barrier);
		{
			vk::SamplerCreateInfo info;
			if (onCreateSample) onCreateSample(info);
			res->sample = dev.createSampler(info);
		}
		{
			vk::ImageViewCreateInfo info({}, res->image, vk::ImageViewType::e2DArray, *format, vk::ComponentMapping(), barrier.subresourceRange);
			res->view = dev.createImageView(info);
		}

		sundry::endSingleTimeCommands(dev, cmd, cmdPool, queue);

		dev.freeMemory(tmpMem);
		dev.destroyBuffer(tmpBuffer);

		return std::make_tuple(true, res);
	}

}
