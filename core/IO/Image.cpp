#include "pch.h"
#include "Image.h"

#include "stb_image.h"

namespace LSIS {
	float* LoadHDRImage(const char* filename, int* width, int* height, int* channels)
	{
		stbi_set_flip_vertically_on_load(true);
		return stbi_loadf(filename, width, height, channels, STBI_rgb_alpha);
	}
}