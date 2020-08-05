#pragma once

#include "Kernels/shared_defines.h"
#include "Compute/Buffer.h"
#include "Compute/Compute.h"

namespace LSIS {

	class BVHBuilderRecursive {
	public:
		BVHBuilderRecursive(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces);
		virtual ~BVHBuilderRecursive();
		
	private:
		std::vector<SHARED::Node> m_nodes;
	};

}