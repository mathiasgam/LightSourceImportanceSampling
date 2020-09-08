#include "pch.h"
#include "Renderer2D.h"

#include "glad/glad.h"

namespace LSIS {

	static const size_t MaxLineCount = 1000;
	static const size_t MaxVertexCount = MaxLineCount;
	static const size_t MaxIndexCount = MaxLineCount;

	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
	};

	struct LineRendererData {
		GLuint VertexArray = 0;
		GLuint VertexBuffer = 0;
		GLuint IndexBuffer = 0;

		uint32_t VertexCount = 0;
		uint32_t IndexCount = 0;

		Vertex* VertexData = nullptr;
		Vertex* VertexDataPtr = nullptr;

		uint32_t* IndexData = nullptr;
		uint32_t* IndexDataPtr = nullptr;

		LineRenderer::Stats RenderStats;
	};

	static LineRendererData s_Data;

	void LineRenderer::Init()
	{
		s_Data.VertexData = new Vertex[MaxVertexCount];
		s_Data.IndexData = new uint32_t[MaxIndexCount];

		glGenVertexArrays(1, &s_Data.VertexArray);
		glGenBuffers(1, &s_Data.VertexBuffer);
		glGenBuffers(1, &s_Data.IndexBuffer);

		glBindVertexArray(s_Data.VertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, s_Data.VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxVertexCount * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, color));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndexCount * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}

	void LineRenderer::Shutdown()
	{
		glDeleteVertexArrays(1, &s_Data.VertexArray);
		glDeleteBuffers(1, &s_Data.VertexBuffer);
		glDeleteBuffers(1, &s_Data.IndexBuffer);
		delete[] s_Data.VertexData;
		delete[] s_Data.IndexData;
	}

	void LineRenderer::BeginBatch()
	{
		s_Data.VertexDataPtr = s_Data.VertexData;
		s_Data.IndexDataPtr = s_Data.IndexData;
	}

	void LineRenderer::EndBatch()
	{
		// Copy vertex data to GPU buffer
		GLsizeiptr vertex_size = (uint8_t*)s_Data.VertexDataPtr - (uint8_t*)s_Data.VertexData;
		glBindBuffer(GL_ARRAY_BUFFER, s_Data.VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_size, s_Data.VertexData);

		// Copy index data to GPU buffer
		GLsizeiptr index_size = (uint8_t*)s_Data.IndexDataPtr - (uint8_t*)s_Data.IndexData;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.IndexBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_size, s_Data.IndexData);
	}

	void LineRenderer::Flush()
	{
		glBindVertexArray(s_Data.VertexArray);
		glDrawArrays(GL_LINES, 0, s_Data.VertexCount);
		glDrawElements(GL_LINES, s_Data.IndexCount, GL_UNSIGNED_INT, nullptr);

		// Update statistics
		s_Data.RenderStats.DrawCount++;

		// reset counts for next batch
		s_Data.IndexCount = 0;
		s_Data.VertexCount = 0;
	}

	void LineRenderer::DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color)
	{
		if (s_Data.VertexCount >= MaxVertexCount || s_Data.IndexCount >= MaxIndexCount) {
			EndBatch();
			Flush();
			BeginBatch();
		}

		s_Data.VertexDataPtr->position = p1;
		s_Data.VertexDataPtr->color = color;
		*(s_Data.IndexDataPtr) = s_Data.VertexCount++;
		s_Data.VertexDataPtr++;
		s_Data.IndexDataPtr++;

		s_Data.VertexDataPtr->position = p2;
		s_Data.VertexDataPtr->color = color;
		*(s_Data.IndexDataPtr) = s_Data.VertexCount++;
		s_Data.VertexDataPtr++;
		s_Data.IndexDataPtr++;

		s_Data.IndexCount += 2;

		s_Data.RenderStats.LineCount++;
	}

	void LineRenderer::DrawLines(const glm::vec3* points, size_t n, const glm::vec3& color)
	{
		if (n < 2) {
			return;
		}

		for (size_t i = 0; i < n - 1; i++) {
			DrawLine(points[i], points[i + 1], color);
		}
	}

	const LineRenderer::Stats& LSIS::LineRenderer::GetStats()
	{
		return s_Data.RenderStats;
	}

	void LineRenderer::ResetStats()
	{
		s_Data.RenderStats.DrawCount = 0;
		s_Data.RenderStats.LineCount = 0;
	}

}