#include <core/object.hpp>

namespace vkd{

struct Comp1 : public Component 
{
	virtual void awake() override
	{
	}
	virtual bool on_init() override
	{
		return false;
	}
	virtual void on_enable() override
	{
	}
	virtual void on_disable() override
	{
	}
	virtual void recreate_swapchain() override
	{
	}
	virtual void draw(vk::CommandBuffer& cmd) override
	{
	}
	virtual void update(float delta) override
	{
	}
	virtual void late_update(float delta) override
	{
	}
	virtual void on_clean_up() override
	{
	}
	virtual void clean_up_pipeline() override
	{
	}
	virtual int64_t idx() override {
		return 1l;
	}
};
struct Comp2 : public Component
{
	virtual void awake() override
	{
	}
	virtual bool on_init() override
	{
		return false;
	}
	virtual void on_enable() override
	{
	}
	virtual void on_disable() override
	{
	}
	virtual void recreate_swapchain() override
	{
	}
	virtual void draw(vk::CommandBuffer& cmd) override
	{
	}
	virtual void update(float delta) override
	{
	}
	virtual void late_update(float delta) override
	{
	}
	virtual void on_clean_up() override
	{
	}
	virtual void clean_up_pipeline() override
	{
	}
	virtual int64_t idx() override {
		return 2l;
	}
};
struct Comp3 : public Component
{
	virtual void awake() override
	{
	}
	virtual bool on_init() override
	{
		return false;
	}
	virtual void on_enable() override
	{
	}
	virtual void on_disable() override
	{
	}
	virtual void recreate_swapchain() override
	{
	}
	virtual void draw(vk::CommandBuffer& cmd) override
	{
	}
	virtual void update(float delta) override
	{
		printf("asdasd\n");
	}
	virtual void late_update(float delta) override
	{
	}
	virtual void on_clean_up() override
	{
	}
	virtual void clean_up_pipeline() override
	{
	}
	virtual int64_t idx() override {
		return 24l;
	}
};
struct Comp4 : public Component
{
	virtual void awake() override
	{
	}
	virtual bool on_init() override
	{
		return false;
	}
	virtual void on_enable() override
	{
	}
	virtual void on_disable() override
	{
	}
	virtual void recreate_swapchain() override
	{
	}
	virtual void draw(vk::CommandBuffer& cmd) override
	{
	}
	virtual void update(float delta) override
	{
	}
	virtual void late_update(float delta) override
	{
	}
	virtual void on_clean_up() override
	{
	}
	virtual void clean_up_pipeline() override
	{
	}
	virtual int64_t idx() override {
		return 36l;
	}
};
}
using namespace vkd;
int main()
{
	auto o = std::make_shared<Object>();

	o->add_comp<Comp4>();
	o->add_comp<Comp3>();
	o->add_comp<Comp1>();
	o->add_comp<Comp2>();

	auto comp3 = o->get_comp<Comp3>();
	comp3.lock()->update(1);

	o->get_comp_raw<Comp3>()->update(1);
	
	o->destroy_comp<Comp2>();

	auto d = std::move(o);

	return 0;
}