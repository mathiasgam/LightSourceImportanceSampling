#include "pch.h"
#include "Core/Timer.h"
#include "PathtracingLayer.h"

#include "Core/Application.h"
#include "Event/Event.h"
#include "Input/KeyCodes.h"

namespace LSIS {

	PathtracingLayer::PathtracingLayer(size_t width, size_t height)
	{
		m_pathtracer = std::make_unique<PathTracer>(width, height);
		SetEventCategoryFlags(EventCategory::EventCategoryApplication | EventCategory::EventCategoryKeyboard);
	}

	PathtracingLayer::~PathtracingLayer()
	{
	}
	void PathtracingLayer::OnUpdate(float delta)
	{
		m_pathtracer->ProcessPass();
		m_pathtracer->UpdateRenderTexture();
	}
	bool PathtracingLayer::OnEvent(const Event& e)
	{
		if (e.GetEventType() == EventType::KeyPressed) {
			auto key_event = (const KeyPressedEvent&)e;
			auto key = key_event.GetKey();
			if (key == KEY_B) {
				std::cout << "PT Event: " << e << std::endl;
				m_pathtracer->Reset();
				return true;
			}
			else if (key == KEY_R) {
				m_pathtracer->Reset();
				std::cout << "PT: Kernels Recompiled!\n";
				return true;
			}
		}
		if (e.GetEventType() == EventType::CameraUpdated) {
			m_pathtracer->ResetSamples();
			auto cam = Application::Get()->GetScene()->GetCamera();
			m_pathtracer->SetCameraProjection( glm::transpose(glm::inverse(cam->GetViewProjectionMatrix())));
			return true;
		}
		return false;
	}
	void PathtracingLayer::OnAttach()
	{
	}
	void PathtracingLayer::OnDetach()
	{
	}
}
