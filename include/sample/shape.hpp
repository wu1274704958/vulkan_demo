#pragma once

#include <glm/glm.hpp>
#include <functional>

#ifndef PI
#define PI 3.14159265358979323846
#endif


struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 uv;
};

namespace shape {

	template<typename Item>
	class Iter {
	public:
		using Opt = std::optional<Item>;
		virtual Opt next() = 0;
	};

	template<typename Item>
	class ForwardIter : public Iter<Item> {
	public:
		int count;
		int current;
	private:
		using R = ForwardIter<Item>::Opt;
		std::function<R (int, int)> apply;
	public:
		ForwardIter(int count, std::function<R (int, int)> callback)
			: apply(callback), count(count){}
		virtual R next() override {
			if (current > count) return std::nullopt;
			return apply(current++, count);
		}
	};

	
	template<
		typename V, typename C, typename U
	>
	class Shape {
		public:
			using ItrV = Iter<V>;
			using ItrC = Iter<C>;
			using ItrU = Iter<U>;
			using ItrVCU = Iter<std::tuple<V, C, U>>;

			virtual std::shared_ptr < ItrV > vertex() = 0;
			virtual std::shared_ptr < ItrC > color() = 0;
			virtual std::shared_ptr < ItrU > uv() = 0;

			virtual std::shared_ptr < ItrVCU > generate_vcu() {
				auto v = (this->vertex()).get();
				auto c = (this->color()).get();
				auto uv = (this->uv()).get();
				ForwardIter<std::tuple<V, C, U>> itr(INT_MAX, [this, v, c, uv](long i, long count) {
					auto _v = v->next();
					auto _c = c->next();
					auto _u = uv->next();
					if (!_v || !_c || !_u) {
						std::optional<std::tuple<V, C, U>> ret = std::nullopt;
						return ret;
					}
					return std::make_optional(std::make_tuple(*_v, *_c, *_u));
				});
				return std::make_shared<Shape<V, C, U>::ItrVCU>(itr);
			}
	};

	//template<
	//	typename V, typename C, typename U
	//>
	//class vcu_itr :public Iter<std::tuple<V, C, U>> {
	//	
	//	public:
	//		Shape<V, C, U>* sp;
	//		std::tuple<V, C, U> v;
	//	public:
	//		vcu_itr(Shape<V,C,U>* s): sp(s){}

	//		vcu_itr::Opt next() override {
	//			auto v = (sp->vertex()).get()->next();
	//			auto c = (sp->color()).get()->next();
	//			auto uv = (sp->uv()).get()->next();
	//			if (!v || !c || !uv) {
	//				return std::nullopt;
	//			}
	//			return std::make_optional(std::make_tuple(*v, *c, *uv));
	//		}
	//};

	

	class Test : public Shape<int, int, int> {
		public:
			virtual std::shared_ptr < ItrV > vertex() override;
			virtual std::shared_ptr < ItrC > color() override;
			virtual std::shared_ptr < ItrU > uv() override;
	};
	
	

	//class Circle : public Shape<glm::vec2, glm::vec2, glm::vec2> {
	//public:
	//	int divin; // Ï¸·Ö³Ì¶È
	//	Circle(int d) : divin(d) {}
	//	Circle() : divin(90){}

	//public:
	//	virtual ItrV vertex() override;
	//	virtual ItrC color() override;
	//	virtual ItrU uv() override;

	//protected:
	//	class itrv : public Iter<glm::vec2> {
	//	private:
	//		int divin;
	//		int i;
	//	public:
	//		itrv(Circle& c): divin(c.divin){}
	//		virtual itrv::Opt next() override;
	//	};
	//};

	//Circle::itrv::Opt Circle::itrv::next() {
	//	if (i > divin) return std::nullopt;
	//	auto x = cosf(2 * PI * (i / divin));
	//	auto y = sinf(2 * PI * (i / divin));
	//	++i;
	//	return std::make_optional(glm::vec2{ x, y });
	//}

	//Circle::ItrV Circle::vertex() {
	//	
	//}
	
}