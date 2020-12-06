#pragma once

#include <memory>

#include "Core.h"
#include "Core/Layer.h"

#include "PathTracer.h"

namespace LSIS {

	class PathtracingLayer : public Layer {
	public:
		PathtracingLayer(size_t width, size_t height);
		virtual ~PathtracingLayer();

		virtual void OnUpdate(float delta) override;
		virtual bool OnEvent(const Event& e) override;
		virtual void OnAttach() override;
		virtual void OnDetach() override;

	private:

		Scope<PathTracer> m_pathtracer;
	};

}