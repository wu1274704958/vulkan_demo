#include <res_loader/res_json.hpp>
#include <res_loader/resource_mgr.hpp>
#include <memory>

namespace vkd
{
	using LoadJsonTy = LoadJson<bool>;

	template <>
	LoadJsonTy::RealRetTy LoadJsonTy::load(gld::FStream* s, const std::string& path, bool printErr)
	{
		auto text = gld::DefResMgr::instance()->load<gld::ResType::text>(s,path);
		if(!text)
			return std::make_tuple(false,nullptr);
		Json::Reader reader;
		auto v = std::make_shared<Json::Value>();
		if (reader.parse(*text, *v, printErr))
		{
			return std::make_tuple(true,v);
		}
		else
		{
			if(printErr)
				printf_s("Load Json parse json error : %s\n", reader.getFormatedErrorMessages().c_str());
			return std::make_tuple(false,nullptr);
		}
	}

	
}