#pragma once

#include "Kernels/shared_defines.h"
#include "Compute/Buffer.h"
#include "Compute/Compute.h"

#include "Kernel.h"

namespace LSIS {

	class BVHBuilder : Kernel {

	public:
		BVHBuilder();
		virtual ~BVHBuilder();

		virtual void Compile() override;

		const TypedBuffer<SHARED::Node> Build(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces);

	private:

	private:
		cl::Program m_program;
		cl::Kernel m_kernel_prepare;
		cl::Kernel m_kernel_scene_bounds;
		cl::Kernel m_kernel_morton_code;
		cl::Kernel m_kernel_sort;
		cl::Kernel m_kernel_hireachy;
	};

}