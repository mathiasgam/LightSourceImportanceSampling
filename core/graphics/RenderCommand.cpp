#include "RenderCommand.h"

#include "glad/glad.h"

namespace LSIS::RenderCommand {

	unsigned int pointBuffer = 0;

	void Init() {
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

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

	void RenderPoints(glm::vec3 position, glm::vec4 color)
	{
	}

}