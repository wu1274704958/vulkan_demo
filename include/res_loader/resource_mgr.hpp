#pragma once

#include <filesystem>
#include <comm.hpp>
#include <glsl_preprocess.hpp>
#include <memory>
#include <fileop.hpp>
#ifdef PF_ANDROID
#include <EGLCxt.h>
#include <android/asset_manager.h>
#ifndef Loge
#define Loge(f,...) __android_log_print(ANDROID_LOG_ERROR,"ResMgr @V@",f,##__VA_ARGS__)
#endif
#endif

#include <res_comm.hpp>
#include <res_cache_mgr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "res_shader.hpp"
#include <type_traits>

#ifdef LoadImage
#undef LoadImage
#endif

namespace gld{   

    struct PerfectUri
    {   
        using CxtTy = std::tuple<std::filesystem::path>;
        static std::string perfect(CxtTy&,const std::string&) noexcept(true);
    };

    struct DefStream : public FStream
    {
		virtual void open(const char*, const char*) override;
		virtual bool good() override;
		virtual void close() override;
		virtual ~DefStream(){}
		virtual size_t read(void* buf, size_t pSize, size_t pCount) override;
		virtual size_t write(const void* buf, size_t pSize, size_t pCount) override;
		virtual bool seek(size_t offset, int posAt);
		virtual size_t tell() const override;
		virtual size_t size() const override;
		virtual void flush() override;
		virtual std::optional<std::vector<char>> read_all() override;
		virtual bool eof() const override;
		virtual void read(std::function<void(const char*, size_t)>, const size_t buf_size = 1048576) override;
		DefStream(){}
		DefStream& operator=(const DefStream& d) = delete;
		DefStream(const DefStream& d) = delete;
		DefStream(DefStream&& d);
		DefStream& operator=(DefStream&& d);

	protected:
		::FILE* file = nullptr;
		size_t file_size = 0;
    };

    template<typename TUP,typename Stream,typename PU,typename ...Plugs>
        requires requires{ 
            PU::CxtTy;
            PU::perfect(std::declval<TUP&>(),std::declval<const std::string&>());
            requires std::derived_from<Stream,FStream>;
        }
    class ResourceMgr{
    public:

		template<ResType Rt,typename Uri, typename ... Args>
		auto load(Uri&& uri,Args&& ... args)
			->typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::RetTy
			requires requires (Uri&& uri, Args&& ... args)
		{
			PU::perfect(std::declval<TUP&>(), std::forward<Uri>(uri));
			MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::load(std::declval<FStream*>(), std::declval<std::string&>(), std::forward<Args>(args)...);
			MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::key_from_args(std::forward<Args>(args)...);
		}
		{
			using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
			using ARGS_T = typename Ty::ArgsTy;
			using RET_T = typename Ty::RetTy;

			auto path = PU::perfect(cxt, std::forward<Uri>(uri));
			auto key_ex = Ty::key_from_args(std::forward<Args>(args)...);
			auto key = path + key_ex;

			if (ResCacheMgr<Plugs...>::instance()->template has<static_cast<size_t>(Rt)>(key))
			{
				return ResCacheMgr<Plugs...>::instance()->template get<static_cast<size_t>(Rt)>(key);
			}

			Stream stream;
			stream.open(path.c_str(),"r");
			if(!stream.good())
				return nullptr;

			auto [success, res] = Ty::load(&stream,path,std::forward<Args>(args)...);

			if (success)
				ResCacheMgr<Plugs...>::instance()->template cache<static_cast<size_t>(Rt)>(key, res);
			return res;
		}

		template<ResType Rt, typename Uri>
		auto load(Uri&& uri)
			->typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::RetTy
			requires requires (Uri&& uri)
		{
			PU::perfect(std::declval<TUP&>(), std::forward<Uri>(uri));
			MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::load(std::declval<FStream*>(), std::declval<std::string&>());
		}
		{
			using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
			using ARGS_T = typename Ty::ArgsTy;
			using RET_T = typename Ty::RetTy;

			auto path = PU::perfect(cxt, std::forward<Uri>(uri));

			if (ResCacheMgr<Plugs...>::instance()->template has<static_cast<size_t>(Rt)>(path))
			{
				return ResCacheMgr<Plugs...>::instance()->template get<static_cast<size_t>(Rt)>(path);
			}

			Stream stream;
			stream.open(path.c_str(), "r");
			if (!stream.good())
				return nullptr;

			auto [success, res] = Ty::load(&stream, path);

			if (success)
				ResCacheMgr<Plugs...>::instance()->template cache<static_cast<size_t>(Rt)>(path, res);
			return res;
		}

		template<ResType Rt, typename Uri>
		decltype(auto) rm_cache(Uri&& uri)
			requires requires (Uri&& uri)
		{
			PU::perfect(std::declval<TUP&>(), std::forward<Uri>(uri));
		}
		{
			using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
			auto key = PU::perfect(cxt, std::forward<Uri>(uri));
			return ResCacheMgr<Plugs...>::instance()->template rm_cache<static_cast<size_t>(Rt)>(key);
		}

		template<ResType Rt,typename Uri,typename ... Args>
		decltype(auto) rm_cache(Uri&& uri,Args&& ... args)
			requires requires (Uri&& uri,Args&& ... args)
		{
            PU::perfect(std::declval<TUP&>(),std::forward<Uri>(uri));
			MapResPlug<static_cast<size_t>(Rt), Plugs...>::type::key_from_args(std::forward<Args>(args)...);
		}
		{
			using Ty = typename MapResPlug<static_cast<size_t>(Rt), Plugs...>::type;
            auto key = PU::perfect(cxt,std::forward<Uri>(uri));
			auto key_ex = Ty::key_from_args(std::forward<Args>(args)...);
            key += key_ex;
			return ResCacheMgr<Plugs...>::instance()->template rm_cache<static_cast<size_t>(Rt)>(key);
		}

        void clear_all()
        {
            ResCacheMgr<Plugs...>::instance()->clear_all();
        }
        
        inline static std::shared_ptr<ResourceMgr<TUP, Stream, PU, Plugs...>> instance()
        {
            return self;
        }

        inline static decltype(auto) create_instance(TUP t)
        {
            self = std::make_shared<ResourceMgr<TUP,Stream,PU,Plugs...>>(std::forward<TUP>(t)); 
            return self;
        }
    protected:
        TUP cxt;
        static std::shared_ptr<ResourceMgr<TUP, Stream, PU, Plugs...>> self;
        ResourceMgr(TUP t) : cxt(t) {}
    };

	template<typename ... Args>
	struct LoadText
	{
		using RetTy = std::shared_ptr<std::string>;
		using ArgsTy = std::tuple<Args...>;
		using RealRetTy = std::tuple<bool,RetTy>;

		static RealRetTy load(FStream*,const std::string&);
	};

	/*struct LoadTextWithGlslPreprocess
	{
		using RetTy = std::shared_ptr<std::string>;
		using ArgsTy = void;
		using RealRetTy = std::tuple<bool,RetTy>;

		static RealRetTy load(VKD_RES_MGR_CXT_PTR_TYPE_WITH_COMMA PathTy, VKD_RES_MGR_KEY_TYPE);
	};

	/*struct StbImage {
		unsigned char* data = nullptr;
		int width = 0;
		int height = 0;
		int channel = 0;
		~StbImage();
	};

	struct LoadImage
	{
		using RetTy = std::shared_ptr<StbImage>;
		using ArgsTy = int;
		using RealRetTy = std::tuple<bool,RetTy>;
		static RealRetTy load(VKD_RES_MGR_CXT_PTR_TYPE_WITH_COMMA PathTy, VKD_RES_MGR_KEY_TYPE_WITH_COMMA int req_comp);
	};

	struct LoadScene
	{
		using RetTy = std::shared_ptr<Assimp::Importer>;
		using ArgsTy = unsigned int;
		using RealRetTy = std::tuple<bool,RetTy>;
		static std::string format_args(ArgsTy flag);
		static ArgsTy default_args();
		static RealRetTy load(VKD_RES_MGR_CXT_PTR_TYPE_WITH_COMMA PathTy, VKD_RES_MGR_KEY_TYPE_WITH_COMMA ArgsTy flag);
	};

	typedef ResourceMgr<'/',
		ResLoadPlugTy<ResType::text, LoadText>,
		ResLoadPlugTy<ResType::glsl,LoadTextWithGlslPreprocess>,
		ResLoadPlugTy<ResType::image,LoadImage>,
		ResLoadPlugTy<ResType::model,LoadScene>,
		ResLoadPlugTy<ResType::spirv_with_meta,vkd::LoadSpirvWithMetaData>> DefResMgr;*/

	typedef ResourceMgr<ResMgrCxtTy,DefStream,PerfectUri,ResLoadPlugTy<ResType::text,LoadText>> DefResMgr;
}

#ifdef PF_ANDROID
#undef Loge
#endif