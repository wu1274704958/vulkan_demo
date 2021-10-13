#pragma once
#include <vector>
#include <optional>


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

	template<typename T,T F, T ...N>
	bool eq_enum(T n)
	{
		if constexpr(sizeof...(N) == 0)
		{ 
			return n == F;
		}else{
			return n == F || eq_enum<T,N...>(n);
		}
	}

	template<typename T, typename ...Args>
	struct VarCache;

	template<typename T>
	struct VarCache<T,std::tuple<>>
	{
	public:
		using IN = std::tuple<>;
		VarCache() {}
		VarCache(std::function<T()> f) {
			constructor = f;
		}
		operator std::shared_ptr<T>()
		{
			if (!ptr)
			{
				ptr = std::shared_ptr(new T(constructor()));
			}
			return ptr;
		}
		void clear()
		{
			ptr.reset();
		}
	private:
		std::function<T()> constructor;
		std::shared_ptr<T> ptr;
	};

	template<typename T,typename ...Args>
	struct VarCache
	{	
	public:
		using IN = std::tuple<Args&...>;
		VarCache(){}
		VarCache(std::function<T(IN&)> f,Args&...args) : constructor(f),in(std::forward<Args&>(args)...) {
		}
		operator std::shared_ptr<T> ()
		{
			if (!ptr && in.has_value())
			{
				ptr = std::shared_ptr<T>(new T(constructor(in.value())));
			}
			return ptr;
		}
		void clear()
		{
			ptr.reset();
		}
	private:
		std::function<T(IN&)> constructor;
		std::shared_ptr<T> ptr;
		std::optional<IN> in;
	};
}