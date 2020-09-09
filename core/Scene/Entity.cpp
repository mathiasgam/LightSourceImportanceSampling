#include "pch.h"
#include "Entity.h"

namespace LSIS {

	Entity::Entity(entt::entity handle, Scene* p_scene) 
		: m_entity_handle(handle), m_scene(p_scene)
	{
	}

}