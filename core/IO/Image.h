#pragma once

namespace LSIS {

	float* LoadHDRImage(const char* filename, int* width, int* height, int* channels);
	void SaveImageFromFloatBuffer(const char* filename, int width, int height, int channels, const float* data);

}