#pragma once

#include "Compute/Compute.h"
#include "Compute/Buffer.h"
#include "Kernel.h"

#include "Kernels/shared_defines.h"

namespace LSIS {

	class BVH : public Kernel {
	public:
		BVH();
		virtual ~BVH();

		void SetGeometryBuffers(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces);
		void SetBVHBuffer(const TypedBuffer<SHARED::Node>& nodes);

		virtual void Compile() override;

		void Trace(const TypedBuffer<SHARED::Ray>& rays, const TypedBuffer<SHARED::Intersection>& intersections);


	private:
		cl::Program m_program;
		cl::Kernel m_kernel;

		cl_uint m_num_nodes = 0;
	};

}