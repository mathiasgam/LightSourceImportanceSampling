#pragma once

#include "Event.h"

namespace LSIS {

	class KeyPressedEvent : public Event {
	public:
		KeyPressedEvent(int key, int scancode, int mods)
			: m_Key(key), m_Scancode(scancode), m_Mods(mods)
		{}

		inline int GetKey() const { return m_Key; }
		inline int GetScancode() const { return m_Scancode; }
		inline int GetMods() const { return m_Mods; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: [" << m_Key << ", " << m_Scancode << ", " << m_Mods << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
	private:
		int m_Key, m_Scancode, m_Mods;
	};

	class  KeyRepeatEvent : public Event {
	public:
		KeyRepeatEvent(int key, int scancode, int mods)
			: m_Key(key), m_Scancode(scancode), m_Mods(mods)
		{}

		inline int GetKey() const { return m_Key; }
		inline int GetScancode() const { return m_Scancode; }
		inline int GetMods() const { return m_Mods; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyRepeatEvent: [" << m_Key << ", " << m_Scancode << ", " << m_Mods << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
			EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
	private:
		int m_Key, m_Scancode, m_Mods;
	};

	class  KeyReleasedEvent : public Event {
	public:
		KeyReleasedEvent(int key)
			: m_Key(key)
		{}

		inline int GetKey() const { return m_Key; }
		
		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: [" << m_Key << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
	private:
		unsigned int m_Key;
	};

	class  KeyTypedEvent : public Event {
	public:
		KeyTypedEvent(int keycode)
			: m_KeyCode(keycode)
		{}

		inline int GetKeyCode() const { return m_KeyCode; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "KeyTypedEvent: [" << m_KeyCode << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

	private:
		int m_KeyCode;
	};
	
}