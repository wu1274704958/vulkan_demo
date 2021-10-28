#pragma once

#include <data_comm.hpp>
#include <res_cache_mgr.hpp>
#include "data_pipeline.hpp"

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

    typedef DataMgr<
        DataLoadPlugTy<DataType::SquareIndices,GenSquareIndices,float,float>,
        DataLoadPlugTy<DataType::SquareVertices,GenSquareVertices>,
        DataLoadPlugTy<DataType::PipelineSimple,vkd::LoadPipelineSimple,vk::Device,vk::RenderPass,const vk::Extent2D&,std::string,std::string,std::unordered_set<uint32_t>,std::function<void(vk::GraphicsPipelineCreateInfo)>>
        > DefDataMgr;
} // namespace gld
