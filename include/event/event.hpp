#pragma once

#include <variant>
#include <tuple>
#include <optional>

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
		KeyRepect, 
	};

	struct WindowReSizeEvent {
		int w;
		int h;
	};

	struct MouseButtonEvent {
		int btn;
		int action;//
		int modsBit;
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
		using Ty = decltype(ET == F::Et ? F::Ty : MapEventTy<ET,S...>::Ty);
	};
	
	template<EventType ET>
	struct MapEventTy<ET>
	{
		using Ty = void;
	};
	
	struct Event {
		EventType type;
		VariantEvent m_event;

		template<EventType E>
		decltype(auto) GetEvent()
		{
			return std::get< MapEventTy<E,
				MapEventUnit<EventType::MouseDown,MouseButtonEvent>,
				MapEventUnit<EventType::MouseUp, MouseButtonEvent>,
				MapEventUnit<EventType::MouseMove, CursorPosEvent>,
				MapEventUnit<EventType::WindowReSize, WindowReSizeEvent>,
				MapEventUnit<EventType::KeyDown, KeyEvent>,
				MapEventUnit<EventType::KeyUp, KeyEvent>,
				MapEventUnit<EventType::KeyRepect, KeyEvent>
			>::Ty>(m_event);
		}
	};

	struct GlfwEventConstructor
	{
		
	};
}