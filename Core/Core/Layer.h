#pragma once

#include "Event/Event.h"

namespace LSIS {

	class Layer {
	public:

		virtual void OnUpdate(float delta) = 0;
		virtual bool OnEvent(const Event& e) { return false; }
		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;

		virtual int GetEventCategoriesFlags() { return EventCategory::None; }

	private:

	};

}