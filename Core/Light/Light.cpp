#include "pch.h"
#include "Light.h"

namespace LSIS {
	Light::Light(glm::vec3 position, glm::vec3 color)
		: m_position(position), m_color(color), m_extra(0.0)
	{
	}
	Light::~Light()
	{
	}
}