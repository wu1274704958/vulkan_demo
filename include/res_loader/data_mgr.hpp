#pragma once

#include <data_comm.hpp>
#include <res_cache_mgr.hpp>

namespace gld
{
    template<typename ...Plugs>
    class DataMgr{
    public:

        template<DataType Rt>
        auto
            load(typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type::ArgsTy args)
            ->typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type::RetTy
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type;
            using ARGS_T = typename Ty::ArgsTy;
            using RET_T = typename Ty::RetTy;

            auto key = Ty::key_from_args(args);

            if(ResCacheMgr<Plugs...>::instance()->template has<static_cast<size_t>(Rt)>(key))
            {
                return ResCacheMgr<Plugs...>::instance()->template get<static_cast<size_t>(Rt)>(key);
            }

            auto [success,res] = Ty::load(std::forward<typename Ty::ArgsTy>(args));

            if(success)
                ResCacheMgr<Plugs...>::instance()->template cache<static_cast<size_t>(Rt)>(key,res);
            return res;
        }

        template<DataType Rt>
        auto load()
            ->typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::RetTy
        {

            static_assert(data_ck::has_default_args_func_vt<typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type>::value,
                "Load plug args type must has default_args function!!!");

            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
            using ARGS_T = typename Ty::ArgsTy;
            using RET_T = typename Ty::RetTy;

            return load<Rt>(Ty::default_args());
        }

        template<DataType Rt,typename ...Args>
        decltype(auto)
            load(Args&&... args)
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type;
            using ARGS_T = typename Ty::ArgsTy;
            using RET_T = typename Ty::RetTy;

            static_assert(data_ck::has_load_func3_vt<Ty,Args...>::value,"This load plug not has tuple load function!!!");

            auto key = Ty::key_from_args(std::make_tuple(std::forward<Args>(args)...));

            if(ResCacheMgr<Plugs...>::instance()->template has<static_cast<size_t>(Rt)>(key))
            {
                return ResCacheMgr<Plugs...>::instance()->template get<static_cast<size_t>(Rt)>(key);
            }

            auto [success,res] = Ty::load(std::make_tuple(std::forward<Args>(args)...));

            if(success)
                ResCacheMgr<Plugs...>::instance()->template cache<static_cast<size_t>(Rt)>(key,res);
            return res;
        }

        template<DataType Rt>
        decltype(auto) rm_cache(typename MapResPlug<static_cast<size_t>(Rt),Plugs...>::type::ArgsTy args)
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;

            auto key = Ty::key_from_args(args);

            return ResCacheMgr<Plugs...>::instance()->template rm_cache<static_cast<size_t>(Rt)>(key);
        }

        template<DataType Rt,typename ...Args>
        decltype(auto) rm_cache(Args&&... args)
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
            
            return rm_cache(std::make_tuple(std::forward<Args>(args)...));
        }

        template<DataType Rt>
        decltype(auto) rm_cache_def()
        {
            using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
            static_assert(data_ck::has_default_args_func_vt<Ty>::value , "must has default_args function!!!");

            return rm_cache(Ty::default_args());
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

    struct GenSquareVertices {
        using RetTy = std::shared_ptr<std::vector<float>>;
        using ArgsTy = std::tuple<float, float>;
        using RealRetTy = std::tuple<bool, RetTy>;
        static std::string key_from_args(ArgsTy args);
        static RealRetTy load(ArgsTy args);
        static ArgsTy default_args();
    };

    struct GenSquareIndices {
        using RetTy = std::shared_ptr<std::vector<int>>;
        using ArgsTy = std::tuple<>;
        using RealRetTy = std::tuple<bool, RetTy>;
        static std::string key_from_args(ArgsTy args);
        static RealRetTy load(ArgsTy args);
        static ArgsTy default_args();
    };

    typedef DataMgr<
        DataLoadPlugTy<DataType::SquareIndices,GenSquareIndices>,
        DataLoadPlugTy<DataType::SquareVertices,GenSquareVertices>
        > DefDataMgr;
} // namespace gld