#pragma once

#include <data_comm.hpp>
#include <res_cache_mgr.hpp>
#include "data_pipeline.hpp"
#include "data_vk_res.hpp"

namespace gld
{
    template<typename ...Plugs>
    class DataMgr{
    public:

        template<DataType Rt,typename ... Args>
        auto load(Args&& ... args)
            ->typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type::RetTy
            requires requires (Args&& ... args)
            {
                 MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::load(std::forward<Args>(args)...);
                 MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::key_from_args(std::forward<Args>(args)...);
            }
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type;
            using ARGS_T = typename Ty::ArgsTy;
            using RET_T = typename Ty::RetTy;

            auto key = Ty::key_from_args(std::forward<Args>(args)...);

            if(ResCacheMgr<Plugs...>::instance()->template has<static_cast<size_t>(Rt)>(key))
            {
                return ResCacheMgr<Plugs...>::instance()->template get<static_cast<size_t>(Rt)>(key);
            }

            auto [success,res] = Ty::load(std::forward<Args>(args)...);

            if(success)
                ResCacheMgr<Plugs...>::instance()->template cache<static_cast<size_t>(Rt)>(key,res);
            return res;
        }

        template<DataType Rt, typename ... Args>
        auto load_not_cache(Args&& ... args)
            ->typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::RetTy
            requires requires (Args&& ... args)
        {
            MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::load(std::forward<Args>(args)...);
        }
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
            using ARGS_T = typename Ty::ArgsTy;
            using RET_T = typename Ty::RetTy;
            auto [success, res] = Ty::load(std::forward<Args>(args)...);
            return res;
        }
		
        template<DataType Rt, typename ... Args>
        decltype(auto) rm_cache(Args&& ... args)
			requires requires (Args&& ... args)
		{
			MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::key_from_args(std::forward<Args>(args)...);
		}
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;

            auto key = Ty::key_from_args(std::forward<Args>(args)...);

            return ResCacheMgr<Plugs...>::instance()->template rm_cache<static_cast<size_t>(Rt)>(key);
        }

        inline static std::shared_ptr<DataMgr<Plugs...>> instance()
        {
            if(!self) 
                self = std::shared_ptr<DataMgr<Plugs...>>(new DataMgr<Plugs...>());
            return self;
        }
        

        void clear_all()
        {
            ResCacheMgr<Plugs...>::instance()->clear_all();
        }
        template<DataType Rt>
        void clear_unused()
        {
            ResCacheMgr<Plugs...>::instance()->template clear_unused<static_cast<size_t>(Rt)>();
        }
protected:
private:    
    inline static std::shared_ptr<DataMgr<Plugs...>> self;

        DataMgr<Plugs...>() {}
    };

    template<typename...Args>
    struct GenSquareVertices{
   
        using RetTy = std::shared_ptr<std::vector<float>>;
        using ArgsTy = std::tuple<Args...>;
        using RealRetTy = std::tuple<bool, RetTy>;
        static std::string key_from_args(Args...);
        static RealRetTy load(Args...);
    };
    template<typename...Args>
    struct GenSquareIndices {
        using RetTy = std::shared_ptr<std::vector<int>>;
        using ArgsTy = std::tuple<Args...>;
        using RealRetTy = std::tuple<bool, RetTy>;
        static std::string key_from_args(Args...);
        static RealRetTy load(Args...);
    };

    
} // namespace gld

#include <data_img_arr.hpp>

namespace gld
{

    typedef DataMgr<
        DataLoadPlugTy<DataType::SquareIndices, GenSquareIndices, float, float>,
        DataLoadPlugTy<DataType::SquareVertices, GenSquareVertices>,
        DataLoadPlugTy<DataType::PipelineSimple, vkd::LoadPipelineSimple, vk::Device, vk::RenderPass, const vk::Extent2D&, std::string, std::string,
        uint32_t, std::unordered_set<uint32_t>, std::vector<uint32_t>, std::function<void(vk::GraphicsPipelineCreateInfo&)>>,
        DataLoadPlugTy<DataType::VkBuffer, vkd::CreateVkBuffer, std::string, vk::PhysicalDevice, vk::Device, vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags>,
        DataLoadPlugTy<DataType::VkImage, vkd::LoadVkImage, std::string, int, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)>, std::function<void(vk::SamplerCreateInfo&)>>,
        DataLoadPlugTy<DataType::VkImageArray, vkd::LoadVkImageArray, std::string, int, vk::PhysicalDevice, vk::Device, vk::CommandPool, vk::Queue, std::function<void(vk::ImageCreateInfo&)>, std::function<void(vk::SamplerCreateInfo&)>>
        > DefDataMgr;
}
