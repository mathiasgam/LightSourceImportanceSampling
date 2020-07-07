#pragma once

#include <memory>

#include "Mesh.h"
#include "Material.h"

namespace LSIS {

	class Object {
	public:

	private:
		std::shared_ptr<Mesh> m_mesh;
		std::shared_ptr<Material> m_material;

	};

}