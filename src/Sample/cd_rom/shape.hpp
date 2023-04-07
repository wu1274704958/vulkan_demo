#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <iterator>
#include <memory>

#ifndef PI
#define PI 3.14159265358979323846
#endif




namespace shape {
	template<typename Item>
	class Iter {
	public:
		using R = std::optional<Item>;
		virtual R next() = 0;
	};

	template<typename Item>
	class ForwardIter : public Iter<Item> {
	public:
		int count;
		int current = 0;
	private:
		using R = typename ForwardIter<Item>::R;
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
				auto v = (this->vertex());
				auto c = (this->color());
				auto uv = (this->uv());
				return std::make_shared<ForwardIter<std::tuple<V, C, U>>>(INT_MAX, [this, v, c, uv](long i, long count) {
					auto _v = v->next();
					auto _c = c->next();
					auto _u = uv->next();
					if (!_v || !_c || !_u) {
						std::optional<std::tuple<V, C, U>> ret = std::nullopt;
						return ret;
					}
					return std::make_optional(std::make_tuple(*_v, *_c, *_u));
				});
			}
	};

	 


	class Circle : public Shape<glm::vec2, glm::vec3, glm::vec2> {
	public:
		int divin; // 细分度
		 Circle(int d) : divin(d) {}
		 Circle() : divin(90){}

	public:
		virtual std::shared_ptr < ItrV > vertex() override {
			return std::make_shared<ForwardIter<glm::vec2>>(this->divin+1, [](int i, int d) {
				if (i == 0)
				{
					return std::optional<glm::vec2>({ 0., 0. });
				}
				auto x = cosf(2. * PI * ((float)(i - 1) / (d-1)));
				auto y = sinf(2. * PI * ((float)(i - 1) / (d-1)));
				return std::optional<glm::vec2>({x,y});
			});
		}
		virtual std::shared_ptr < ItrC > color() override {
			return std::make_shared<ForwardIter<glm::vec3>>(this->divin+1, [](int i, int d) {
				return std::optional<glm::vec3>({ 0.,0.,-1. });
			});
		}
		virtual std::shared_ptr < ItrU > uv() override {
			return std::make_shared<ForwardIter<glm::vec2>>(this->divin+1, [](int i, int d) {
				auto x = i == 0? 0. : cosf(2 * PI * ((float)(i - 1) / (d - 1)));
				auto y = i == 0? 0. : sinf(2 * PI * ((float)(i - 1) / (d - 1)));
				return std::optional<glm::vec2>({ (x + 1) / 2, (y + 1) / 2 });
			});
		}

	};
}