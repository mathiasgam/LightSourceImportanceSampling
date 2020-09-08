#pragma once

#include <cinttypes>

#include "glad/glad.h"
#include "CL/cl.hpp"

namespace LSIS {

	class Texture {
		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
	};

	class Texture2D :public Texture{
	public:
		Texture2D(size_t width, size_t height);
		virtual ~Texture2D();

		virtual void Bind()const override;
		virtual void UnBind()const override;

	private:
		uint32_t m_texture_id;
		size_t m_width;
		size_t m_height;
	};

	

}