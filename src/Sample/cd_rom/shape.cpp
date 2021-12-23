#include <memory>
#include <optional>
#include <sample/shape.hpp>
namespace shape {

	std::shared_ptr<Test::ItrV> Test::vertex() {
		ForwardIter<int> v(100, [](int i, int c) {
			return std::make_optional(i);
		});
		return std::make_shared<std::remove_cvref_t<decltype(v)>>(v);
	}
	std::shared_ptr<Test::ItrC> Test::color() {
		ForwardIter<int> v(100, [](int i, int c) {
			return std::make_optional(i);
		});
		return std::make_shared<std::remove_cvref_t<decltype(v)>>(v);
	}
	std::shared_ptr<Test::ItrU> Test::uv() {
		ForwardIter<int> v(100, [](int i, int c) {
			return std::make_optional(i);
		});
		return std::make_shared<std::remove_cvref_t<decltype(v)>>(v);
	}

}