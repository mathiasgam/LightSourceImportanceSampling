#pragma once

#include <vector>
#include <memory>

#include "mesh/Mesh.h"

namespace LSIS {


	class GeometryBuffer {
	public:
		GeometryBuffer(std::shared_ptr<Mesh> mesh);
		virtual ~GeometryBuffer();

		void Bind();
		inline unsigned int GetNumFaces() const { return m_num_faces; }

	private:
		unsigned int m_num_faces = 0;
		unsigned int m_ebo = 0;
		unsigned int m_vbo = 0;
		unsigned int m_vao = 0;
	};


}