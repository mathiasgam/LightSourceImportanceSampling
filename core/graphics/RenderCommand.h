#pragma once

#include "GeometryBuffer.h"

namespace LSIS {
	namespace RenderCommand {

		void Clear();
		void RenderGeometryBuffer(const std::shared_ptr<GeometryBuffer>& buffer);

	}
}