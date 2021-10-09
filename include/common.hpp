#pragma once
#include <vector>


namespace vkd{

	inline bool eq(const char* a,const char* b)
	{
		return strcmp(a,b) == 0;
	}

	template<typename V, template <typename T1, typename Alloc = std::allocator<T1>> class T>
	bool IsContain(const T<V>& vec, const T<V>& inclusion)
	{
		int same_count = 0;
		for (auto& i : inclusion)
		{
			bool has = false;
			for (auto& v : vec)
			{
				if (eq(i, v))
				{
					has = true;
					++same_count;
					break;
				}
			}
			if (same_count == inclusion.size()) return true;
			if (!has) return false;
		}
		return true;
	}

	template<typename V, typename V1,template <typename T1, typename Alloc = std::allocator<T1>> class T>
	bool IsContain(const T<V1>& vec, const T<V>& inclusion, V V1::*f )
	{
		int same_count = 0;
		for (auto& i : inclusion)
		{
			bool has = false;
			for (auto& v : vec)
			{
				if (eq(i, v.*f))
				{
					has = true;
					++same_count;
					break;
				}
			}
			if (same_count == inclusion.size()) return true;
			if (!has) return false;
		}
		return true;
	}

	template<typename V, typename V1, template <typename T1, typename Alloc = std::allocator<T1>> class T>
	bool IsContain(const T<V1>& vec, const T<V>& inclusion, std::function<V(const V1&)> f)
	{
		int same_count = 0;
		for (auto& i : inclusion)
		{
			bool has = false;
			for (auto& v : vec)
			{
				if (eq(i, f(v)))
				{
					has = true;
					++same_count;
					break;
				}
			}
			if (same_count == inclusion.size()) return true;
			if (!has) return false;
		}
		return true;
	}
}