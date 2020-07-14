#include "Object.h"
#include "graphics/RenderCommand.h"

namespace LSIS {


	Object::Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, Transform transform)
		: m_transform(transform), m_material(material)
	{
		m_mesh = std::make_shared<GeometryBuffer>(mesh);
	}

	Object::~Object()
	{
	}

	void Object::Render(const glm::mat4& cam_matrix)
	{
		m_material->Bind(m_transform, cam_matrix);
		m_mesh->Bind();
		RenderCommand::RenderGeometryBuffer(m_mesh);
	}

}