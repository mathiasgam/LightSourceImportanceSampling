#pragma once

#include "glm.hpp"

#include "Core.h"
#include "Mesh/Mesh.h"
#include "Scene/Material.h"

namespace LSIS {

	struct TransformComponent {
		glm::mat4 Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform) 
			: Transform(transform) {}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () { return Transform; }
	};

	struct MeshComponent {
		Ref<Mesh> mesh;
		Ref<Material> material;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Ref<Mesh> mesh, Ref<Material> material)
			: mesh(mesh), material(material) {}
	};

	struct LightComponent {
		glm::vec4 color;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(const glm::vec4& color)
			: color(color) {}
	};


}