#pragma once

#include <core/object.hpp>
#include <core/component.hpp>

namespace vkd
{
	struct Vertex;

	struct SkyBox : public Component
	{
		SkyBox(std::string path);
		std::shared_ptr<Component> clone() const override;
		void awake() override;
		static void onCreatePipeline(vk::GraphicsPipelineCreateInfo& info);
		void recreate_swapchain() override;
		void attach_scene(const std::weak_ptr<Scene>& scene) override;
		bool on_init() override {return true;} 
		void on_clean_up() override {}
		void draw(vk::CommandBuffer& cmd) override{};
		int64_t idx() override{ return static_cast<int64_t>(CompIdx::Pipeline) - 1; }

		static std::shared_ptr<std::vector<Vertex>> vertices;
		static std::shared_ptr<std::vector<uint16_t>> indices;

	protected:
		std::string path;
	};
}
