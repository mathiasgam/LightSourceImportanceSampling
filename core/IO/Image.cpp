#include "pch.h"
#include "Image.h"

#include "stb_image.h"
#include "stb_image_write.h"

namespace LSIS {
	float* LoadHDRImage(const char* filename, int* width, int* height, int* channels)
	{
		stbi_set_flip_vertically_on_load(true);
		return stbi_loadf(filename, width, height, channels, STBI_rgb_alpha);
	}

	float color_correction(float color, float gamma, float exposure) {
		// reinhard tone mapping
		const float mapped = color / (color + exposure);
		// gamma correction 
		return fminf(1.0f, fmaxf(0.0f,pow(mapped, 1.0f / gamma)));
	}

	void SaveImageFromFloatBuffer(const char* filename, int width, int height, int channels, const float* data)
	{
		const int num_pixels = width * height;
		uint8_t* pixels = new uint8_t[num_pixels * channels];

		const float gamma = 2.2;
		const float exposure = 1.0f;

		for (int i = 0; i < num_pixels; i++) {
			pixels[i * 4 + 0] = 255 * color_correction(data[i * 4 + 0], gamma, exposure);
			pixels[i * 4 + 1] = 255 * color_correction(data[i * 4 + 1], gamma, exposure);
			pixels[i * 4 + 2] = 255 * color_correction(data[i * 4 + 2], gamma, exposure);
			pixels[i * 4 + 3] = 255;
		}

		stbi_flip_vertically_on_write(true);
		stbi_write_png(filename, width, height, channels, pixels, width * channels);
	}
}