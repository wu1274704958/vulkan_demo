#pragma once

#include <filesystem>
#include <comm.hpp>
#include <functional>
#ifdef PF_ANDROID
struct EGLCxt;
#endif
namespace gld
{
    
    enum class ResType{
        text = 0x0,
        json,
        image,
        model,
        font,
        spirv_with_meta,

    };

#ifdef PF_ANDROID
    using ResMgrCxtTy = std::tuple<std::shared_ptr<EGLCxt>>;
    using ResMgrKeyTy = std::string;
#else
    using ResMgrCxtTy = std::tuple<std::filesystem::path>;
    using ResMgrKeyTy = std::string;
#endif

    struct FStream
    {
    public:
        virtual void open(const char*, const char*) = 0;
        virtual bool good() = 0;
        virtual void close() = 0;
        virtual ~FStream(){}
        virtual size_t read(void* buf,size_t pSize,size_t pCount) = 0;
        virtual size_t write(const void* buf,size_t pSize, size_t pCount) = 0;
        virtual bool seek(size_t offset,int posAt) = 0;
        virtual size_t tell() const = 0;
        virtual size_t size() const = 0;
        virtual void flush() = 0;
        virtual bool eof() const = 0;
        virtual std::optional<std::vector<char>> read_all() = 0;
        virtual void read(std::function<void(const char*,size_t)>, const size_t buf_size) = 0;
    };
    
    template<typename T, typename R>
	concept HasRealRetTy = std::is_same_v<typename T::RealRetTy, std::tuple<bool, R>>;

	template<ResType ty, template<typename ...As> class T, typename ... ARGS>
	requires requires (ARGS... args) {
		T<ARGS...>::RetTy;
		T<ARGS...>::ArgsTy;
		T<ARGS...>::load(std::declval<FStream*>(),std::declval<std::string&>(),std::forward<ARGS>(args)...);
		//T<ARGS...>::key_from_args(args...);
		requires HasRealRetTy<T<ARGS...>, typename T<ARGS...>::RetTy>;
	}
	struct ResLoadPlugTy
	{
		constexpr static size_t res_type = static_cast<size_t>(ty);
		using type = T<ARGS...>;

		using Ret = typename T<ARGS...>::RetTy;
		using Args = typename T<ARGS...>::ArgsTy;
	};
	

    template<size_t Rt,typename ...Ts>
    struct MapResPlug;

    template<size_t Rt,typename Fir,typename ...Ts>
    struct MapResPlug<Rt,Fir,Ts...>
    {
        constexpr static decltype(auto) func()
        {
            if constexpr (Rt == Fir::res_type)
            {
                using T = typename Fir::type;
                return std::declval<T>();
            }
            else
            {
                using T = typename MapResPlug<Rt, Ts...>::type;
                if constexpr (std::is_same_v<T, void>)
                {
                    static_assert("Error Type!!!");
                }
                return std::declval<T>();
            }
        }
        using type = typename std::remove_reference_t<decltype(func())>;
    };

    template<size_t Rt>
    struct MapResPlug<Rt>
    {
        using type = void;
    };

    template<size_t Rt,size_t Idx,typename Fir,typename ...Ts>
    constexpr size_t get_res_idx_inside()
    {
        if constexpr(Rt == Fir::res_type)
        {
            return Idx;
        }else{
            if constexpr( sizeof...(Ts) > 0 )
            {
                return get_res_idx_inside<Rt,Idx + 1,Ts...>();
            }
        }
    }

    template<size_t Rt,typename ...Ts>
    constexpr size_t get_res_idx()
    {
        return get_res_idx_inside<Rt,0,Ts...>();
    }


    template<typename ...T>
    std::tuple<bool,T...> make_result(bool s,T&& ...t)
    {
        return std::make_tuple(s,std::forward<T>(t)...);
    }

} // namespace gld

#undef RES_MGR_CXT
#undef VKD_RES_MGR_KEY1
#undef VKD_RES_MGR_KEY2