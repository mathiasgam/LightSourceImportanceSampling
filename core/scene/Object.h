#pragma once

#include <memory>

#include "mesh/Mesh.h"
#include "Material.h"
#include "Transform.h"

namespace LSIS {

	class Object {
	public:

		Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform);
		virtual ~Object();

		void Render(const glm::mat4& cam_matrix);

	private:
		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Material> m_material;
		Transform m_transform; 
	};

}