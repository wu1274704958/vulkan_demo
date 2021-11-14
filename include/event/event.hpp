#pragma once

#include <variant>
#include <tuple>
#include <optional>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace vkd {
	class SampleRender;
}

namespace vkd::evt{

	enum class EventType
	{
		None,
		WindowReSize,
		MouseDown,
		MouseUp,
		MouseMove,
		KeyDown,
		KeyUp,
		KeyRepeat, 
		Click
	};

	struct WindowReSizeEvent {
		int w;
		int h;
	};

	struct MouseButtonEvent {
		int btn;
		int action;//
		int modsBit;
		double x,y;
	};

	struct CursorPosEvent {
		double x;
		double y;
	};

	struct KeyEvent {
		int key;
		int scancode;
		int action;
		int modsBit;
	};

	using VariantEvent = std::variant<WindowReSizeEvent, MouseButtonEvent, CursorPosEvent, KeyEvent>;

	template<EventType ET,typename T>
	struct MapEventUnit
	{
		using Ty = T;
		static constexpr EventType Et = ET;
	};

	template<EventType ET, typename ... S>
	struct MapEventTy;

	template<EventType ET, typename F, typename ...S>
	struct MapEventTy<ET,F,S...>
	{
		constexpr static decltype(auto) func()
		{
			if constexpr (ET == F::Et)
			{
				return std::declval<F::Ty>();
			}
			else
			{
				return std::declval<MapEventTy<ET,S...>::Ty>();
			}
		}
		using Ty = typename std::remove_reference_t<decltype(func())>;
	};
	
	template<EventType ET>
	struct MapEventTy<ET>
	{
		using Ty = std::nullopt_t;
	};
	
	struct Event {
		EventType type;
		VariantEvent m_event;

		template<EventType E>
		using EvtTy = typename MapEventTy<E,
			MapEventUnit<EventType::MouseDown, MouseButtonEvent>,
			MapEventUnit<EventType::MouseUp, MouseButtonEvent>,
			MapEventUnit<EventType::MouseMove, CursorPosEvent>,
			MapEventUnit<EventType::WindowReSize, WindowReSizeEvent>,
			MapEventUnit<EventType::KeyDown, KeyEvent>,
			MapEventUnit<EventType::KeyUp, KeyEvent>,
			MapEventUnit<EventType::KeyRepect, KeyEvent>,
			MapEventUnit<EventType::Click,MouseButtonEvent>
		>::Ty;

		template<typename T>
		 const T& GetEvent() const
		{
			return std::get<T>(m_event);
		}
		 template<EventType E>
		 const EvtTy<E>& GetEvent() const
		 {
			 return std::get<EvtTy<E>>(m_event);
		 }
	};

	class EventDispatcher {
	public:
		virtual bool dispatchEvent(const Event&) { return false; }
	};

	template<typename T,typename ...Args>
	struct EventConstructor
	{
		virtual bool init(T* p, Args...args)
		{
			t = p;
			return false;
		}
		void dispatchEvent(const Event& e)
		{
			t->dispatchEvent(e);
		}
	protected:
		T *t;
	};

	struct GlfwEventConstructor : public EventConstructor<SampleRender,GLFWwindow*>
	{
		virtual bool init(SampleRender* p, GLFWwindow*) override;
		static void WindowReSize(GLFWwindow* window, int w, int h);
		static void WindowMouseButton(GLFWwindow*, int, int, int);
		static void WindowCursorPos(GLFWwindow*,double,double);
		static void WindowKey(GLFWwindow*, int, int, int, int);
	protected:
		std::optional<MouseButtonEvent> mouseBtnPressed;
		bool isMouseMoving = false;
		static constexpr double MinStartMoveDist = 9.0;
	};
	//template<>
	
}