#pragma once

#include "Scene.h"
#include "entt.hpp"

namespace LSIS {

	class Entity {
	public:
		Entity() = default;
		Entity(const Entity& other) = default;

		Entity(entt::entity handle, Scene* p_scene);

		uint32_t GetID() const { return (uint32_t) m_entity_handle; }

		template<typename T>
		bool HasComponent() {
			return m_scene->Reg().has<T>(m_entity_handle);
		}

		template<typename T, typename ... Args>
		T& AddComponent(Args&&... args) {
			CORE_ASSERT(!HasComponent<T>(), "Entity already have component!");
			return m_scene->Reg().emplace<T>(m_entity_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent() {
			CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_scene->Reg().get<T>(m_entity_handle);
		}

		template<typename T>
		T& RemoveComponent() {
			CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_scene->Reg().remove<T>(m_entity_handle);
		}

	private:
		entt::entity m_entity_handle{ 0 };
		Scene* m_scene = nullptr;
	};

}