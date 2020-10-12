#include "pch.h"
#include "LightStructure.h"

namespace LSIS {

	inline glm::vec3 convert(cl_float4 in) {
		return glm::vec3(in.x, in.y, in.z);
	}

	TypedBuffer<cl_float> LSIS::build_power_sampling_buffer(const SHARED::Light* lights, const size_t num_lights)
	{
		if (num_lights == 0) {
			return TypedBuffer<cl_float>();
		}

		cl::CommandQueue queue = Compute::GetCommandQueue();

		float* areas = new float[num_lights];
		float* cdf = new float[2 * num_lights];

		float sum_area = 0.0f;
		for (uint32_t i = 0; i < num_lights; i++) {
			SHARED::Light light = lights[i];
			cl_float4 color = light.intensity;

			const glm::vec3 tangent = convert(light.tangent);
			const glm::vec3 bitangent = convert(light.bitangent);

			const float area = glm::length(glm::cross(tangent, bitangent)) * 0.5f;
			areas[i] = area;
			sum_area += area;
		}

		printf("Total Area: %f\n", sum_area);

		const float inv_sum = 1.0f / sum_area;

		float accum = 0.0f;
		//cdf[0] = 0.0f;
		for (uint32_t i = 0; i < num_lights; i++) {
			const float pdf = areas[i] * inv_sum;
			cdf[i] = accum;
			cdf[i + num_lights] = 1.0f * inv_sum; // write pdf
			accum += pdf;
			//cdf[i] = (powers[i]*inv_sum) + cdf[i - 1];
		}

		TypedBuffer<cl_float> sampling_buffer = TypedBuffer<cl_float>(Compute::GetContext(), CL_MEM_READ_ONLY, num_lights * 2);
		CHECK(queue.enqueueWriteBuffer(sampling_buffer.GetBuffer(), CL_TRUE, 0, sizeof(cl_float) * (num_lights * 2), (void*)cdf));


		// Cleanup
		delete[] areas;
		delete[] cdf;

		return sampling_buffer;
	}

	inline glm::vec3 get_vec3(cl_float4 vec) {
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	inline glm::vec3 get_position(SHARED::Light& light) {
		return get_vec3(light.position);
	}

	float sqr(float x) {
		return x * x;
	}

	/**
	intensity = accumulated rgb values of the light intensities
	diagonal = diagonal length of the AABB of the two light clusters
	half_angle = half angle of the bounding cone, when using oriented lights
	scaling = controls the relative scalling between spatial and directional similarty, diagonal for the scenes AABB of oriented lights and 0 for omni and directional lights
	*/
	float similarity(float intensity, float diagonal, float half_angle, float scaling) {
		return intensity * (sqr(diagonal) + sqr(scaling) * sqr(1.0f - cos(half_angle)));
	}
}