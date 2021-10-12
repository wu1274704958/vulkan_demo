#include <Sample/render.hpp>

class Quad : public vkd::SampleRender {
	void onInit() override {

	}
	void onCreate() override {

	}
};

void main()
{
	auto quad = new Quad();
	quad->init(800,600);
	
}