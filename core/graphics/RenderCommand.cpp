#include "RenderCommand.h"

#include "glad/glad.h"

namespace LSIS::RenderCommand {

	unsigned int pointBuffer = 0;

	void Init() {
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		SetPointSize(4.0f);
	}

	void SetPointSize(float size)
	{
		glPointSize(size);
	}

	void SetViewPort(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
		glViewport(x, y, width, height);
	}

	void Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RenderGeometryBuffer(const std::shared_ptr<Mesh>& buffer)
	{
		glDrawElements(GL_TRIANGLES, buffer->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
	}

	void RenderPointMesh(const std::shared_ptr<PointMesh>& mesh)
	{
		glDrawArrays(GL_POINTS, 0, mesh->GetNumPoints());
	}

}