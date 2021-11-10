#include <res_loader/data_vk_res.hpp>
#include <sundry.hpp>
#include <res_loader/resource_mgr.hpp>
#include <stb_image.h>
#include <common.hpp>
#include <vulkan/vulkan.hpp>

namespace gld::vkd {

	

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

	inline bool createTempBuf(vk::PhysicalDevice phyDev, vk::Device dev, vk::DeviceSize size,void *data, vk::Buffer& buf, vk::DeviceMemory& mem,vk::DeviceSize offset = 0)
	{
		if (!createBuffer(phyDev, dev, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buf, mem))
			return false;
		void* dst = dev.mapMemory(mem, offset, size);
		memcpy(dst,data, size);
		dev.unmapMemory(mem);
		return true;
	}

	inline bool createImage(vk::PhysicalDevice phyDev, vk::Device dev,uint32_t w,uint32_t h,vk::Format format,vk::ImageTiling tiling,vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlagBits memProp,vk::Image& img,vk::DeviceMemory& mem, std::function<void(vk::ImageCreateInfo&)> onCreateImage)
	{
		vk::ImageCreateInfo info;
		info.extent = vk::Extent3D(w,h,1);
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
		if(!img) return false;
		mem = sundry::allocVkMemory(dev,phyDev,memProp,img);
		if (!mem) {
			dev.destroyImage(img);
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////Image.../////////////////////////////////////////////////////////////////////

	VkdImage::~VkdImage()
	{
		if (!device) return;
		if (view) device.destroyImageView(view);
		if (mem) device.freeMemory(mem);
		if (image) device.destroyImage(image);
		if (sample) device.destroySampler(sample);
	}

	LoadVkImageTy::RealRetTy LoadVkImageTy::load(const std::string& key, int flag, vk::PhysicalDevice phyDev, vk::Device dev, vk::CommandPool cmdPool, vk::Queue queue, std::function<void(vk::ImageCreateInfo&)> onCreateImage, std::function<void(vk::SamplerCreateInfo&)> onCreateSample)
	{
		std::shared_ptr<StbImage> img = DefResMgr::instance()->load<ResType::image>(key,flag);
		if(!img) return std::make_tuple(false,nullptr);
		if (flag == STBI_rgb_alpha) img->channel = 4;
		auto format = wws::map_enum<wws::ValList<int,1,3,4>,
			wws::ValList<vk::Format,
			vk::Format::eR8Unorm, vk::Format::eR8G8B8Unorm, vk::Format::eR8G8B8A8Unorm
			>>(img->channel);
		if(!format) return std::make_tuple(false, nullptr);
		
		vk::DeviceSize size = img->width * img->height * img->channel;
		vk::Buffer tmpBuffer;vk::DeviceMemory tmpMem;
		if (!createTempBuf(phyDev, dev, size, (void*)img->data, tmpBuffer, tmpMem))
			return std::make_tuple(false, nullptr);

		auto res = std::make_shared<VkdImage>();

		if (!createImage(phyDev, dev, img->width, img->height, *format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal,
			res->image, res->mem,onCreateImage))
			return std::make_tuple(false, nullptr);

		res->device = dev;
		res->format = *format;
		res->tiling = vk::ImageTiling::eOptimal;

		vk::CommandBuffer cmd = sundry::beginSingleTimeCommands(dev,cmdPool);
		vk::ImageMemoryBarrier barrier;
		barrier.image = res->image;
		barrier.oldLayout = vk::ImageLayout::eUndefined; 
		barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,0,1,0,1);
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,vk::PipelineStageFlagBits::eTransfer,(vk::DependencyFlagBits)0, {}, {},barrier);
		vk::BufferImageCopy region;
		region.imageExtent = vk::Extent3D(img->width,img->height,1);
		region.imageSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor,0,0,1);
		cmd.copyBufferToImage(tmpBuffer,res->image,vk::ImageLayout::eTransferDstOptimal,region);
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eNoneKHR;
		cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, (vk::DependencyFlagBits)0, {}, {}, barrier);
		{
			vk::SamplerCreateInfo info;
			if(onCreateSample) onCreateSample(info);
			res->sample = dev.createSampler(info);
		}
		{
			vk::ImageViewCreateInfo info({},res->image,vk::ImageViewType::e2D,*format, vk::ComponentMapping(),barrier.subresourceRange);
			res->view = dev.createImageView(info);
		}

		sundry::endSingleTimeCommands(dev,cmd,cmdPool,queue);

		dev.freeMemory(tmpMem);
		dev.destroyBuffer(tmpBuffer);

		return std::make_tuple(true, res);
	}

	std::string LoadVkImageTy::key_from_args(const std::string& str, int f)
	{
		return sundry::format_tup('#',str,f);
	}
	std::string LoadVkImageTy::key_from_args(const std::string& str, int f, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)>,
		std::function<void(vk::SamplerCreateInfo&)>)
	{
		return sundry::format_tup('#', str, f);
	}
	
}