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

int main()
{
	shape::Test test;
	auto t = test.generate_vcu().get();
	while (true)
	{
		auto d = t->next();
		if (!d) { break; }
		auto [a, b, c] = d.value();
		printf("a: %d, b: %d, c: %c", a, b, c);
	}
	return 0;
}

