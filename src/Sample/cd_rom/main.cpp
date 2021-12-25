#include <core/object.hpp>

#include <comm_comp/transform.hpp>
#include <comm_comp/pipeline.hpp>
#include <sample/render.hpp>
#include <res_loader/resource_mgr.hpp>
#include <res_loader/data_mgr.hpp>
#include <res_loader/data_pipeline.hpp>
#include <json.hpp>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <comm_comp/showcase.hpp>
#include <comm_comp/scene.hpp>
#include <generator/Generator.hpp>
#include <comm_comp/render.hpp>
#include <misc_comp/MiscComp.hpp>
#include <sundry.hpp>
#include <comm_comp/renderpass.hpp>

#include <event/event.hpp>

#include <sample/shape.hpp>
#include <ranges>

int main()
{
	shape::Circle c(180);
	auto t = c.generate_vcu();
	auto v = t->vector();
	std::ranges::for_each(v, [](auto d) {
		auto [a, b, c] = d;
		printf("v: { %f, %f }, c: { %f, %f, %f }, uv: { %f, %f } \r\n",
			a.x, a.y,
			b.x, b.y, b.z,
			c.x, c.y);
	});
	return 0;
}

