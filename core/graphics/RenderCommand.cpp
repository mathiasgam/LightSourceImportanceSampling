#include "RenderCommand.h"

#include "glad/glad.h"

namespace LSIS::RenderCommand {
	
	void Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderGeometryBuffer(const std::shared_ptr<GeometryBuffer>& buffer)
	{
		glDrawElements(GL_TRIANGLES, buffer->GetNumFaces(), GL_UNSIGNED_INT, nullptr);
	}

}