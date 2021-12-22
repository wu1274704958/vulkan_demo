#pragma once
#include <tuple>
#include <exception>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <serialization.hpp>
#include <random>
#include <glm/gtc/random.hpp>

#ifdef PF_ANDROID
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES/gl.h>
#include <GLES3/gl32.h>
#endif

#include <vulkan/vulkan.hpp>

namespace sundry
{
    template<typename GenTy = float>
    GenTy normal_dist(GenTy x,GenTy u = 0.0,GenTy o = 1.0)
    {
        return static_cast<GenTy>(1.0f / glm::sqrt(2 * glm::template pi<GenTy>() * o) * glm::exp(-glm::pow(x - u, 2.f) / (2.0f * glm::pow(o, 2.f))));
    }

    template<size_t I,typename ...Args>
    void format_tup_sub(std::tuple<Args...>& tup,std::string& res,char sepa)
    {
        if constexpr( I < std::tuple_size_v<std::tuple<Args...>>)
        {
            using type = std::decay_t<std::remove_reference_t<decltype(std::get<I>(tup))>>;
            if constexpr(std::is_same_v<std::string,type> || std::is_same_v<const char*,type> || std::is_same_v<char,type>)
            {
                res += std::get<I>(tup);
            }else{
                res += wws::to_string(std::get<I>(tup));
            }
            if constexpr( I + 1 < std::tuple_size_v<std::tuple<Args...>>)
            {
                res += sepa;
                format_tup_sub<I + 1>(tup,res,sepa);
            }
        }
    }

    template<typename ...Args>
    std::string format_tup(std::tuple<Args...>& tup,char sepa,char per = '\0',char back = '\0')
    {
        std::string res;
        if(per != '\0') res += per;
        format_tup_sub<0>(tup,res,sepa);
        if(back != '\0') res += back;
        return res;
    }


	template<char P = '\0', char B = '\0', typename ...Args>
	std::string format_tup(char sepa,Args&&... args)
	{
		std::string res;
        auto tup = std::make_tuple(std::forward<Args>(args)...);
		if constexpr(P != '\0'){ res += P; }
		format_tup_sub<0>(tup, res, sepa);
		if constexpr(B != '\0'){ res += B; }
		return res;
	}

    inline float rd_0_1()
    {
        return glm::linearRand<float>(0.f, 1.f);
    }

    inline void screencoord_to_ndc(int width, int height, float x, float y, float* nx, float* ny,bool flipY = true) 
    {
        *nx = (float)x / (float)width * 2 - 1;
        *ny = ((float)y / (float)height * 2 - 1) * (flipY ? -1.f : 1.f); // reverte y axis
    }

    inline void
        normalized2d_to_ray(float nx, float ny,glm::mat4 inverse_mvp,glm::vec3 camerapos, glm::vec3& raypos, glm::vec3& raydir) {
        // 世界坐标 - 视图矩阵 - 透视矩阵 - 透视除法 - ndc
        // 要得到反转得到世界坐标，先需要视图矩阵和透视矩阵的反转矩阵

        // ndc 坐标系是左手坐标系，所以近平面的 z 坐标为远平面的 z 坐标要小
        glm::vec4 nearpoint_ndc(nx, ny, -1.f, 1.f);
        glm::vec4 nearpoint_world = inverse_mvp * nearpoint_ndc;

        // 消除矩阵反转后，透视除法的影响
        nearpoint_world /= nearpoint_world.w;

        raypos = glm::vec3(nearpoint_world);
        raydir = raypos - camerapos;
        raydir = glm::normalize(raydir);
    }

	vk::CommandBuffer beginSingleTimeCommands(vk::Device device,vk::CommandPool cmdPool);
	void endSingleTimeCommands(vk::Device device,vk::CommandBuffer cb, vk::CommandPool cmdPool,vk::Queue queue, std::function<void()> onWait = {});
    uint32_t findMemoryType(vk::PhysicalDevice phyDev, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    template<typename T>
    inline vk::DeviceMemory allocVkMemory(vk::Device dev, vk::PhysicalDevice phyDev, vk::MemoryPropertyFlagBits memProp,T t,vk::DeviceSize offset = 0)
	requires requires{
		requires std::is_same_v<std::remove_cvref_t<T>, vk::Image> || std::is_same_v<std::remove_cvref_t<T>, vk::Buffer>;
	}
    {
        vk::MemoryRequirements memReq;
		if constexpr(std::is_same_v<std::remove_cvref_t<T>, vk::Image>)
		{
		    memReq = dev.getImageMemoryRequirements(t);
        }
        else 
        if constexpr (std::is_same_v<std::remove_cvref_t<T>, vk::Buffer>)
        {
            memReq = dev.getBufferMemoryRequirements(t);
        }
		vk::MemoryAllocateInfo info(memReq.size, findMemoryType(phyDev, memReq.memoryTypeBits, memProp));
        auto mem = dev.allocateMemory(info);
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, vk::Image>)
		{
			dev.bindImageMemory(t,mem, offset);
		}
		else
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, vk::Buffer>)
		{
            dev.bindBufferMemory(t,mem, offset);
		}
        return mem;
    }

	bool createBuffer(vk::PhysicalDevice phyDev, vk::Device device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memProperty, vk::Buffer& buffer, vk::DeviceMemory& mem);
	bool createImage(vk::PhysicalDevice phyDev, vk::Device dev, uint32_t w, uint32_t h, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlagBits memProp, vk::Image& img, vk::DeviceMemory& mem, std::function<void(vk::ImageCreateInfo&)> onCreateImage,uint32_t arrayLayers = 1,
        vk::ImageType ty = vk::ImageType::e2D);

    template<typename T>
    union VkObjToId
    {
        VkObjToId(T t)
        {
	        obj = t;
        }
        T obj;
        size_t id;
    };
} // namespace sundry
