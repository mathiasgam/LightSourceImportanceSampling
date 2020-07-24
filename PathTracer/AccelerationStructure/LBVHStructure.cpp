#include "pch.h"
#include "LBVHStructure.h"

namespace LSIS {
	
	LBVHStructure::LBVHStructure()
	{
	}

	LBVHStructure::~LBVHStructure()
	{
	}

	void LBVHStructure::Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices)
	{
		std::cout << "Building LBVH\n";
		std::cout << "Num Vertices: " << num_vertices << std::endl;
		std::cout << "Num Indices: " << num_indices << std::endl;
	}

	void LBVHStructure::TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer)
	{
	}

	void LBVHStructure::CompileKernels()
	{
	}

	void LBVHStructure::LoadGeometryBuffers()
	{	
	}

	void LBVHStructure::LoadBVHBuffer()
	{
	}
}