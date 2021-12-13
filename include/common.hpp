#pragma once
#include <vector>
#include <optional>
#include <comm.hpp>

namespace wws{

	template<typename T,typename Vec>
	requires requires(Vec& v)
	{
		requires std::is_same_v<decltype(v[std::declval<int64_t>()]),T&>;
	}
	void swap(Vec& vec,int a,int b)
	{
		T t = vec[a];
		vec[a] = vec[b];
		vec[b] = t;
	}

	template<typename T,typename Vec>
	requires requires(Vec& v)
	{
		requires std::is_same_v<decltype(v[std::declval<int64_t>()]), T&>;
	}
	void adjust_down(Vec& vec,int pIdx,int len)
	{
		T tmp = vec[pIdx];
		int cIdx = pIdx * 2 + 1; //默认 left

		while (cIdx < len)
		{
			if(cIdx + 1 < len && vec[cIdx + 1] > vec[cIdx])//判断是否有右节点 且 右节点大于 左节点
				++cIdx; //换成右节点
			if(tmp > vec[cIdx])
				break;
			vec[pIdx] = vec[cIdx];
			pIdx = cIdx;
			cIdx = cIdx * 2 + 1;
		}
		vec[pIdx] = tmp;
	}

	template<typename T, typename Vec>
	requires requires(Vec& v,int len)
	{
		len = v.size();
		requires std::is_same_v<decltype(v[std::declval<int64_t>()]), T&>;
	}
	void sort_heap(Vec& vec)
	{
		int len = vec.size();
		for(int i = (len - 2) / 2;i >= 0;--i )
			adjust_down<T>(vec,i,len);
		for(int i = len - 1;i > 0;--i)
		{
			swap<T>(vec,0,i);
			adjust_down<T>(vec,0,i);
		}
	}

	template<typename T, typename Vec,typename Comp>
	requires requires(Vec& v, const Comp& comp)
	{
		requires std::is_same_v<decltype(comp.cmp(std::declval<const T>(), std::declval<const T>())),bool>;
		requires std::is_same_v<decltype(v[std::declval<int64_t>()]), T&>;
	}
	void adjust_down(Vec& vec, int pIdx, int len, const Comp& comp)
	{
		T tmp = vec[pIdx];
		int cIdx = pIdx * 2 + 1; //默认 left

		while (cIdx < len)
		{
			if (cIdx + 1 < len && !comp.cmp(vec[cIdx + 1], vec[cIdx]))//判断是否有右节点 且 右节点大于 左节点
				++cIdx; //换成右节点
			if (!comp.cmp(tmp,vec[cIdx]))
				break;
			vec[pIdx] = vec[cIdx];
			pIdx = cIdx;
			cIdx = cIdx * 2 + 1;
		}
		vec[pIdx] = tmp;
	}

	template<typename T, typename Vec,typename Comp>
	requires requires(Vec& v, int len, const Comp& comp)
	{
		len = v.size();
		requires std::is_same_v<decltype(comp.cmp(std::declval<const T>(), std::declval<const T>())), bool>;
		requires std::is_same_v<decltype(v[std::declval<int64_t>()]), T&>;
	}
	void sort_heap(Vec& vec, const Comp& comp)
	{
		int len = vec.size();
		for (int i = (len - 2) / 2; i >= 0; --i)
			adjust_down<T>(vec, i, len,comp);
		for (int i = len - 1; i > 0; --i)
		{
			swap<T>(vec, 0, i);
			adjust_down<T>(vec, 0, i,comp);
		}
	}
}

namespace vkd {
	enum class EngineState : uint32_t{
		Uninitialized,
		Initialized,
		Running,
		Stoped,
		Destroyed
	};

	template<typename T>
	struct Clone
	{
		virtual std::shared_ptr<T> clone() const = 0;
	};
}
