#include "pch.h"
#include "PointRenderer.h"

#include "glad/glad.h"

namespace LSIS {


	static const size_t MaxPointCount = 1000;
	static const size_t MaxVertexCount = MaxPointCount;
	static const size_t MaxIndexCount = MaxPointCount;

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	struct RendererData {
		GLuint VertexArray = 0;
		GLuint VertexBuffer = 0;

		uint32_t PointCount = 0;

		Vertex* PointBuffer = nullptr;
		Vertex* PointBufferPtr = nullptr;

		PointRenderer::Stats RenderStats;
	};

	static RendererData s_Data{};

	void LSIS::PointRenderer::Init()
	{
		s_Data.PointBuffer = new Vertex[MaxVertexCount];

		glGenVertexArrays(1, &s_Data.VertexArray);
		glBindVertexArray(s_Data.VertexArray);

		glGenBuffers(1, &s_Data.VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));

		glBindVertexArray(0);
	}

	void LSIS::PointRenderer::Shutdown()
	{
		glDeleteVertexArrays(1, &s_Data.VertexArray);
		glDeleteBuffers(1, &s_Data.VertexBuffer);
		delete[] s_Data.PointBuffer;
	}

	void LSIS::PointRenderer::BeginBatch()
	{
		s_Data.PointBufferPtr = s_Data.PointBuffer;
	}

	void LSIS::PointRenderer::EndBatch()
	{
		GLsizeiptr size = (uint8_t*)s_Data.PointBufferPtr - (uint8_t*)s_Data.PointBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, s_Data.PointBuffer);
	}

	void LSIS::PointRenderer::Flush()
	{
		glBindVertexArray(s_Data.VertexArray);
		glDrawArrays(GL_POINTS, 0, s_Data.PointCount);
		s_Data.RenderStats.DrawCount++;

		s_Data.PointCount = 0;
	}

	void LSIS::PointRenderer::DrawPoint(const glm::vec3& position, const glm::vec3& color)
	{
		if (s_Data.PointCount >= MaxPointCount) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		s_Data.PointBufferPtr->position = position;
		s_Data.PointBufferPtr->color = color;
		s_Data.PointBufferPtr++;

		s_Data.PointCount++;
		s_Data.RenderStats.PointCount++;
	}

	const PointRenderer::Stats& PointRenderer::GetStats()
	{
		return s_Data.RenderStats;
	}

	void LSIS::PointRenderer::ResetStats()
	{
		s_Data.RenderStats.DrawCount = 0;
		s_Data.RenderStats.PointCount = 0;
	}

}