#pragma once

#include <string>

namespace LSIS {

	enum class EventType {
		None = 0,

		WindowResized,
		WindowMoved,
		WindowFocus,

		KeyPressed,
		KeyReleased,
		
		MouseMoved,
		MouseClicked,
		MouseDragged,
	};

#define BIT(x) 1 << x

	enum class EventCategory {
		None = 0,

		Input = BIT(1),
		Window = BIT(2)
	};

	class Event {
	public:
		virtual EventType GetEventType() = 0;
		virtual EventCategory GetEventCategory() = 0;
		virtual std::string GetName() = 0;
	private:
	};

}