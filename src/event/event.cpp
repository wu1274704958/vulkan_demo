#include <event/event.hpp>
#include <sample/render.hpp>
#include <GLFW/glfw3.h>
#include <math.h>
namespace vkd::evt {

	double length(const glm::vec<2, double>& a, const glm::vec<2, double>& b)
	{
		return ::pow(a.x - b.x,2.0) + ::pow(a.y - b.y,2.0);
	}

	double length(double ax,double ay,double bx,double by)
	{
		return ::pow(ax - bx, 2.0) + ::pow(ay - by, 2.0);
	}

	bool GlfwEventConstructor::isMouseBtnPress(int btn)
	{
		return mouseBtnPressed && mouseBtnPressed->btn == btn;
	}
	int GlfwEventConstructor::getMouseBtnPress()
	{
		return mouseBtnPressed ? mouseBtnPressed->btn : -1;
	}

	bool GlfwEventConstructor::init(SampleRender* p, GLFWwindow* window) {
		EventConstructor<SampleRender,GLFWwindow*>::init(p,window);

		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, WindowReSize);
		glfwSetMouseButtonCallback(window, WindowMouseButton);
		glfwSetCursorPosCallback(window,WindowCursorPos);
		glfwSetKeyCallback(window,WindowKey);
		glfwSetScrollCallback(window,WindowScroll);
		return true; 
	}

	void GlfwEventConstructor::WindowReSize(GLFWwindow* window, int w, int h)
	{
		auto ptr = (GlfwEventConstructor*)glfwGetWindowUserPointer(window);
		Event e;
		e.m_event = WindowReSizeEvent{w,h};
		e.type = EventType::WindowReSize;
		ptr->dispatchEvent(e);
	}

	void GlfwEventConstructor::WindowMouseButton(GLFWwindow* window, int button, int action, int mods)
	{
		auto ptr = (GlfwEventConstructor*)glfwGetWindowUserPointer(window);
		Event e;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		e.m_event = MouseButtonEvent{ button,action,mods,x,y};
		double v;
		switch (action)
		{
		case GLFW_PRESS:
			e.type = EventType::MouseDown;
			ptr->mouseBtnPressed = MouseButtonEvent{ button,action,mods,x,y };
			break;
		case GLFW_RELEASE:
			e.type = EventType::MouseUp;
			ptr->dispatchEvent(e);
			ptr->mouseBtnPressed = std::nullopt;
			if (!ptr->isMouseMoving)
			{
				e.type = EventType::Click;
				ptr->dispatchEvent(e);
			}
			ptr->isMouseMoving = false;
			return;
			break;
		default:
			break;
		}
		ptr->dispatchEvent(e);
	}

	void GlfwEventConstructor::WindowCursorPos(GLFWwindow* window, double x, double y)
	{
		auto ptr = (GlfwEventConstructor*)glfwGetWindowUserPointer(window);
		if(!ptr->mouseBtnPressed)return;
		if(ptr->isMouseMoving || length(x, y, ptr->mouseBtnPressed->x, ptr->mouseBtnPressed->y) >= MinStartMoveDist)
		{ 
			ptr->isMouseMoving = true;
			Event e;
			e.type = EventType::MouseMove;
			ptr->mouseBtnPressed->x = x;
			ptr->mouseBtnPressed->y = y;
			e.m_event = ptr->mouseBtnPressed.value();
			ptr->dispatchEvent(e);
		}
	}

	void GlfwEventConstructor::WindowKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto ptr = (GlfwEventConstructor*)glfwGetWindowUserPointer(window);
		Event e;
		e.m_event = KeyEvent{ key,scancode,action,mods };
		switch (action)
		{
		case GLFW_PRESS:
			e.type = EventType::KeyDown;
			break;
		case GLFW_RELEASE:
			e.type = EventType::KeyUp;
			break;
		case GLFW_REPEAT:
			e.type = EventType::KeyRepeat;
			break;
		default:
			break;
		}
		ptr->dispatchEvent(e);
	}

	void GlfwEventConstructor::WindowScroll(GLFWwindow* window, double x, double y)
	{
		auto ptr = (GlfwEventConstructor*)glfwGetWindowUserPointer(window);
		Event e;
		e.m_event = ScrollEvent{ x,y };
		e.type = EventType::Scroll;
		ptr->dispatchEvent(e);
	}


}