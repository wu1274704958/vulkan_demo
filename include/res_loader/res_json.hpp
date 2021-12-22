#pragma once
#include <json/json.h>
#include "res_comm.hpp"

namespace vkd{

template<typename ... Args>
struct LoadJson
{
	using RetTy = std::shared_ptr<Json::Value>;
	using ArgsTy = std::tuple<Args...>;
	using RealRetTy = std::tuple<bool, RetTy>;
	static RealRetTy load(gld::FStream*, const std::string&, bool printErr = true);
};

}