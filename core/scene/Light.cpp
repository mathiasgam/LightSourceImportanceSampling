#include "Light.h"

namespace LSIS {
	Light::Light(glm::vec3 position, glm::vec3 color)
		: m_position(position,0), m_color(color,0), m_extra(0.0)
	{
	}
	Light::~Light()
	{
	}
}