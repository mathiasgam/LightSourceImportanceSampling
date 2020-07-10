#pragma once

#include "Event.h"


namespace LSIS {

	class MouseMovedEvent : public Event 
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_X(x), m_Y(y)
		{}

		inline float GetX() const { return m_X; }
		inline float GetY() const { return m_Y; }

		std::string ToString() const override {
			std::stringstream ss;

			ss << "MouseMoveEvent: [" << m_X << ", " << m_Y << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_X, m_Y;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float x, float y)
			: m_X(x), m_Y(y)
		{}

		inline float GetX() const { return m_X; }
		inline float GetY() const { return m_Y; }

		std::string ToString() const override {
			std::stringstream ss;

			ss << "MouseScrolledEvent: [" << m_X << ", " << m_Y << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_X, m_Y;
	};

	class MouseButtonPressedEvent : public Event
	{
	public:
		MouseButtonPressedEvent(int button, int mod)
			: m_Button(button), m_Mod(mod)
		{}

		inline int GetButton() const { return m_Button; }
		inline int GetMod() const { return m_Mod; }

		std::string ToString() const override {
			std::stringstream ss;

			ss << "MouseButtonPressedEvent: [" << m_Button << ", " << m_Mod << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_Button, m_Mod;
	};

	class MouseButtonReleasedEvent : public Event
	{
	public:
		MouseButtonReleasedEvent(int button, int mod)
			: m_Button(button), m_Mod(mod)
		{}

		inline int GetButton() const { return m_Button; }
		inline int GetMod() const { return m_Mod; }

		std::string ToString() const override {
			std::stringstream ss;

			ss << "MouseButtonReleasedEvent: [" << m_Button << ", " << m_Mod << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		int m_Button, m_Mod;
	};

}