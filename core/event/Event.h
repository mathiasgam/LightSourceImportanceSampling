#pragma once

#include "Core.h"

#include <string>
#include <sstream>

namespace LSIS {

	enum class EventType {
		None = 0,

		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,

		PathDrop,

		KeyPressed, 
		KeyReleased,
		KeyTyped,

		MouseMoved,
		MouseScrolled,
		MouseButtonPressed,
		MouseButtonRepeat,
		MouseButtonReleased
	};

	enum EventCategory {
		None = 0,
		EventCategoryApplication =  BIT(0),
		EventCategoryInput =        BIT(1),
		EventCategoryKeyboard =     BIT(2),
		EventCategoryMouse =        BIT(3),
		EventCategoryMouseButton =  BIT(4)
	};

#define EVENT_CLASS_TYPE(type)\
	static EventType GetStaticType() {return EventType::##type; }\
	virtual EventType GetEventType() const override { return GetStaticType(); }\
	virtual const char* GetName() const override {return #type; }

#define EVENT_CLASS_CATEGORY(category)\
	virtual int GetCategoryFlags() const override { return category; }


	class Event
	{
		friend class EventDispathcer;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory catergory) {
			return GetCategoryFlags() & catergory;
		}

		inline bool IsHandled() { return m_Handled; }

	protected:
		bool m_Handled = false;
	};

	class  EventDispathcer
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispathcer(Event& event)
			: m_Event(event) {
		}
		
		template<typename T>
		bool Dispatch(EventFn<T> func) {
			if (m_Event.GetEventType() == T::GetStaticType()) {
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}