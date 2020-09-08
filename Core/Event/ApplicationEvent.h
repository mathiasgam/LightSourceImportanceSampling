#pragma once

#include "Event.h"
#include "Scene/Camera.h"

#include <vector>

namespace LSIS {

	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: m_Width(width), m_Height(height)
		{}

		inline unsigned int GetWidth() const { return m_Width; }
		inline unsigned int GetHeight() const { return m_Height; }

		std::string ToString() const override {
			std::stringstream ss;
			ss << "WindowResizeEvent: [" << m_Width << ", " << m_Height << "]";
			return ss.str();
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		unsigned int m_Width, m_Height;
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowFocusEvent : public Event {
	public:
		WindowFocusEvent() {}

		EVENT_CLASS_TYPE(WindowFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowLostFocusEvent : public Event {
	public:
		WindowLostFocusEvent() {}

		EVENT_CLASS_TYPE(WindowLostFocus)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class PathDropEvent : public Event {
	public:
		PathDropEvent(int count, const char** paths)
			: m_paths() 
		{
			//CH_CORE_ASSERT(count > 0, "count has to be positive!");
			m_paths.resize(count);
			for (int i = 0; i < count; i++) {
				m_paths[i] = std::string(paths[i]);
			}
		}

		std::vector<std::string> const& GetPaths() const { return m_paths; }

		EVENT_CLASS_TYPE(PathDrop)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		std::vector<std::string> m_paths;
	};

	class WindowDragEvent : public Event {
	public:
		WindowDragEvent(int offsetX, int offsetY)
			: m_offsetX(offsetX), m_offsetY(offsetY) {}

		EVENT_CLASS_TYPE(WindowMoved)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		int m_offsetX;
		int m_offsetY;
	};

	class CameraUpdatedEvent : public Event {
	public:
		CameraUpdatedEvent(Ref<Camera> cam)
			: m_cam(cam) {}

		EVENT_CLASS_TYPE(CameraUpdated)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	private:
		Ref<Camera> m_cam;
	};
}