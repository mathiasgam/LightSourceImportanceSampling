#include "Object.h"

namespace LSIS {


	Object::Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform)
		: m_mesh(mesh), m_material(material), m_transform(transform)
	{
	}

	Object::~Object()
	{
	}

	void Object::Render(const glm::mat4& cam_matrix)
	{
		m_material->Bind(m_transform, cam_matrix);
		m_mesh->Render();
	}

}