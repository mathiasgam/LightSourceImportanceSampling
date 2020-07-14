#pragma once

#include <memory>

#include "graphics/GeometryBuffer.h"
#include "Material.h"
#include "Transform.h"

namespace LSIS {

	class Object {
	public:

		Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform);
		virtual ~Object();

		void Render(const glm::mat4& cam_matrix);

	private:
		Transform m_transform; 
		std::shared_ptr<Material> m_material;
		std::shared_ptr<GeometryBuffer> m_mesh;
	};

}