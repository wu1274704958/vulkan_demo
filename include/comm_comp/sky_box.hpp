#pragma once

#include <core/object.hpp>
#include <core/component.hpp>

namespace vkd
{
	struct Vertex;

	struct SkyBox : public Component
	{
		void awake() override;
		static void onCreatePipeline(vk::GraphicsPipelineCreateInfo& info);

		static std::shared_ptr<std::vector<Vertex>> vertices;
		static std::shared_ptr<std::vector<uint16_t>> indices;
	};
}
