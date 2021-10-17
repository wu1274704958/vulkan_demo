#include <Sample/render.hpp>
#include <json.hpp>

namespace wws {

	template <typename S, std::size_t ...N>
	constexpr std::array<std::remove_reference_t<decltype(S::get()[0])>, sizeof...(N)>
		prepare_std_arr_impl(S, std::index_sequence<N...>)
	{ 
		return {{ S::get()[N] ... }};
	}


	template <typename S>
	constexpr decltype(auto) prepare_std_arr(S s) {
		return prepare_std_arr_impl(s,
			std::make_index_sequence< sizeof(S::get())>{});
	}

}
#define PREPARE_STD_ARR(s)													\
    (::wws::prepare_std_arr([]{												\
        struct tmp {                                                        \
            static constexpr decltype(auto) get() { return s; }             \
        };                                                                  \
        return tmp{};                                                       \
    }())) 

class Quad : public vkd::SampleRender {
public:
	Quad(bool enableValidationLayers, const char* sample_name) : vkd::SampleRender(enableValidationLayers,sample_name)
	{}
private:
	void onInit() override {
		
	}
	void onReCreateSwapChain() override {

	}
	void onRealDraw(vk::CommandBuffer& cmd) override {

	}
	void onCleanUp() override {

	}
	void onCleanUpPipeline() override {

	}
};

template<typename T,size_t N,std::array<T,N> Arr>
struct MyStruct
{
	template<size_t I>
	static constexpr T get()
	{
		return Arr[I];
	}
};

template<size_t I,typename T,size_t N>
constexpr T getIdx(std::array<T, N> arr)
{
	return arr[I];
}

template<char V>
struct Val {
	static constexpr char v = V;
};


void main()
{
	constexpr int a = MyStruct<int,2,std::array<int,2>{1,2}>::get<0>();
	constexpr int b = MyStruct<int,2,std::array<int, 2>{1,2}>::get<1>();


	constexpr auto e = MyStruct<const char, 17, PREPARE_STD_ARR(
	R"(sadd
	asda
	asda)"
	)>::get<1>();


	

	printf("%d %d %c %c",a, b,e,Val<getIdx<0>(PREPARE_STD_ARR("def"))>::v);
	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	quad->cleanUp();
	delete quad;
}


