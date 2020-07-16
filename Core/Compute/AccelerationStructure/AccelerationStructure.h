#pragma once

#include "Compute/Buffer.h"
#include "Kernels/shared_defines.h"

#include "Mesh/Mesh.h"

namespace LSIS::Compute {

	using RayBuffer = TypedBuffer<SHARED::Ray>;
	using IntersectionBuffer = TypedBuffer<SHARED::Intersection>;

	class AccelerationStructure {
	public:

		virtual void Build(std::vector<Mesh>& meshes) = 0;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) = 0;

	private:
	};

}