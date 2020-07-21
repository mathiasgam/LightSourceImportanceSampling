#pragma once

#include "Event/Event.h"

namespace LSIS {

	class Layer {
	public:

		virtual void OnUpdate(float delta) {};
		virtual bool OnEvent(const Event& e) { return false; }
		virtual void OnAttach() {};
		virtual void OnDetach() {};

		void SetEventCategoryFlags(int flags) { m_category_flags = flags; }
		int GetEventCategoriesFlags() { return m_category_flags; }

	private:
		int m_category_flags = EventCategory::None;
	};

}