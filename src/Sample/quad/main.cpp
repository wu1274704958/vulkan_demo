#include <Sample/render.hpp>

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
};

void main()
{
	auto quad = new Quad(true,"Quad");
	quad->init(800,600);
	quad->mainLoop();
	printf("Ok!!!");
}