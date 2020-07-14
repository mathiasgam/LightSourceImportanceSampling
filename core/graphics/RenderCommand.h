#pragma once

#include "GeometryBuffer.h"

namespace LSIS {
	namespace RenderCommand {

		void Init();

		void Begin();
		void End();

		void SetViewPort(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

		void Clear();
		void RenderGeometryBuffer(const std::shared_ptr<GeometryBuffer>& buffer);

	}
}