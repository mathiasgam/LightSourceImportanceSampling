#pragma once

#include "mesh/Mesh.h"

namespace LSIS {
	namespace RenderCommand {

		void Init();

		void SetViewPort(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

		void Clear();
		void RenderGeometryBuffer(const std::shared_ptr<Mesh>& buffer);

	}
}