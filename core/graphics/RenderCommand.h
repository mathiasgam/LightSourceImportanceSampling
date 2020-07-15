#pragma once

#include "Mesh/Mesh.h"
#include "Mesh/PointMesh.h"

namespace LSIS {
	namespace RenderCommand {

		void Init();

		void SetViewPort(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

		void Clear();
		void RenderGeometryBuffer(const std::shared_ptr<Mesh>& buffer);
		void RenderPointMesh(const std::shared_ptr<PointMesh>& mesh);

	}
}