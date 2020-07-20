#include "Texture.h"

namespace LSIS {


	Texture2D::Texture2D(size_t width, size_t height)
	{
		glGenTextures(1, &m_texture_id);
		m_width = width;
		m_height = height;

		//generate the texture ID
		glGenTextures(1, &m_texture_id);
		//binnding the texture
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
		//regular sampler params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//need to set GL_NEAREST
		//(not GL_NEAREST_MIPMAP_* which would cause CL_INVALID_GL_OBJECT later)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size[0], size[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture2D::~Texture2D()
	{
		if (m_texture_id)
			glDeleteTextures(1, &m_texture_id);
	}

	void Texture2D::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}

	void Texture2D::UnBind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}