#include "commonCL.h"

inline float3 ColorFromNormal(float3 normal) {
	float3 col = normalize(normal).xyz * 0.5f + 0.5f;
	return col.xyz;
}

inline float3 ColorFromPosition(float3 position) {
	float3 remain = remainder(position, 10.0f) * 0.1f;
	return (float3)(remain.xyz);
}

inline float3 GetBackground(float3 dir) {
	const float3 UP = (float3)(0, 1, 0);
	const float d = 1.0f - max(dot(dir, UP), 0.0f);

	const float3 sky = (float3)(0.8f, 1.0f, 1.0f);
	const float3 ground = (float3)(0.55f, 0.40f, 0.17f);

	if (d == 1.0f) {
		return (float3)(0.2f, 0.2f, 0.2f);
	}

	return mix(sky, ground, d * d);
}

float2 direction_to_hdri(float3 d) {
	float x = (atan2(d.x, d.z) + M_PI_F) / (M_PI_F * 2.0f);
	float y = (asin(d.y) + (M_PI_F / 2.0f)) / M_PI_F;
	return (float2)(x, y);
}

// From https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/RadeonProRender-Baikal/master/Baikal/Kernels/CL/sampling.cl
inline int lower_bound(__global float const* values, int n, float value)
{
	int count = n;
	int b = 0;
	int it = 0;
	int step = 0;

	while (count > 0)
	{
		it = b;
		step = count / 2;
		it += step;
		if (values[it] < value)
		{
			b = ++it;
			count -= step + 1;
		}
		else
		{
			count = step;
		}
	}

	return b;
}


int select_light(__global const float* cdf, int num_lights, float r, float* pdf_out) {
	// TODO do binary search in the cdf of the lights

	const int index = max(1, lower_bound(cdf, num_lights, r)) - 1;

	if (index < 0 || index >= num_lights) {
		printf("index: %d, num_lights: %d, r: %f\n", index, num_lights, r);
		*pdf_out = 1.0f;
		return 0;
	}
	/*
	uint index = num_lights - 1;
	for (uint i = 1; i < num_lights; i++) {
		if (cdf[i] > r) {
			index = i - 1;
			break;
		}
	}
	*/
	const float pdf = cdf[index + 1] - cdf[index];
	if (pdf < 0.0f || pdf > 1.0f) {
		printf("PDF: %f, index: %d, num_lights: %d, cdf[i]: %f, cdf[i+1]: %f\n", pdf, index, num_lights, cdf[index], cdf[index+1]);
	}
	//*pdf = 1.0f; // pdf is canceling out with power / area calculation, so all lights uses their material emission
	// only works for area lights. need to calculate per light pdf if both point and area lights are used
	*pdf_out = pdf;
	return index;
}

float sqr(float x) {
	return x * x;
}

inline float3 sample_triangle(float3 ab, float3 ac, float r1, float r2) {
	if (r1 + r2 > 1.0f) {
		r1 = 1.0f - r1;
		r2 = 1.0f - r2;
	}
	return ab * r1 + ac * r2;
}

const sampler_t sampler_in = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

__kernel void ProcessBounce(
	IN_VAL(uint, num_samples),
	IN_VAL(uint, num_lights),
	IN_VAL(uint, num_pixels),
	IN_VAL(uint, multi_sample_count),
	IN_VAL(uint, seed),
	IN_BUF(Intersection, hits),
	IN_BUF(GeometricInfo, geometrics),
	IN_BUF(Light, lights),
	IN_BUF(Material, materials),
	IN_BUF(float, light_power_cdf),
	OUT_BUF(float3, results),
	OUT_BUF(float3, throughputs),
	OUT_BUF(int, states),
	OUT_BUF(float3, light_contribution),
	OUT_BUF(Ray, bounce_rays),
	OUT_BUF(Ray, shadow_rays),
	__read_only image2d_t texture
) {
	int id = get_global_id(0);
	uint rng = hash2(hash2(id) ^ hash1(seed));

	//barrier(CLK_GLOBAL_MEM_FENCE);

	if (id < num_samples) {
		GeometricInfo geometric = geometrics[id];
		Intersection hit = hits[id];

		int state = states[id];

		// choose light
		//uint i = random_uint(&rng, num_lights);
		//float pdf = inverse(num_lights);
		float r = rand(&rng);
		float pdf;
		uint i = select_light(light_power_cdf, num_lights, r, &pdf);

		//printf("l: %d, %f\n", i, r);

		float3 result = results[id];
		float3 throughput = throughputs[id];

		// iterate over all lights
		//for (uint i = 0; i < num_lights; i++){
		if (state & STATE_ACTIVE) {
			// process miss
			if (hit.hit == 0) {
				float2 coord = direction_to_hdri(geometric.incoming.xyz);
				result += read_imagef(texture, sampler_in, coord).xyz * throughput;
				//result += GetBackground(geometric.incoming.xyz) * throughput;
				state = STATE_INACTIVE;
			}
			else {
				//throughput *= max(-dot(geometric.incoming.xyz,geometric.normal.xyz),0.0f); 
				Light light = lights[i];
				float3 light_pos = light.position.xyz;
				if (light.position.w == 1.0f) {
					const float r1 = rand(&rng);
					const float r2 = rand(&rng);
					light_pos += sample_triangle(light.tangent.xyz, light.bitangent.xyz, r1, r2);
				}

				Material material = materials[hit.material_index];
				float3 diffuse = material.diffuse.xyz;
				float3 emission = material.emission.xyz;


				throughput *= diffuse * M_1_PI_F;

				const float3 diff = light_pos - geometric.position.xyz;
				const float dist = length(diff);
				const float dist_inv = inverse(dist);
				const float3 dir = diff * dist_inv;

				const float attenuation = inv_sqr(dist + 1.0f);

				float d = dot(geometric.normal.xyz, dir);

				if (state & STATE_FIRST) {
					// first sample does not have the next event estimation
					result += emission * throughput;
					state = STATE_ACTIVE;
				}
				else {
					// Next event estimation is also a sample, 
					// so the average of next event and emissive hit is used to combine the two into one
					result += emission * throughput * 0.5f;
				}

				d *= max(-dot(light.direction.xyz, dir), 0.0f);

				// calculate the lights contribution
				float3 L = light.intensity.xyz * max(d, 0.0f) * attenuation * 0.5f;
				//result += L * material.diffuse.xyz * throughput;
				//sample.result.xyz += L * throughput * material.diffuse.xyz;

				float3 lift = geometric.normal.xyz * 0.0001f;

				shadow_rays[id] = CreateRay(geometric.position.xyz + lift, dir, 0.0001f, dist - 0.001f);
				light_contribution[id] = (L * throughput) * inverse(pdf);

				/*
				// Russian roulette
				float pdf_russian = (throughput.x + throughput.y + throughput.z) / 3.0f;
				if (rand(&rng) > pdf_russian) {
					state = STATE_INACTIVE;
				}
				else {
					throughput = throughput * inverse(pdf_russian);
				}
				*/
				
				
				float3 out_dir = sample_hemisphere_cosine(&rng, geometric.normal.xyz);
				bounce_rays[id] = CreateRay(geometric.position.xyz + lift, out_dir, 0.0001f, 1000.0f);
			}
		}

		states[id] = state;
		results[id] = result;
		throughputs[id] = throughput;
	}
}

__kernel void shade_occlusion(
	IN_BUF(int, hits),
	IN_BUF(int, states),
	IN_BUF(float3, contributions),
	IN_VAL(uint, num_samples),
	OUT_BUF(float3, results)
) {
	const int id = get_global_id(0);

	if (id < num_samples) {

		const int hit = hits[id];
		const int state = states[id];
		const float3 result = results[id];
		const float3 contribution = contributions[id];

		if (hit == -1 && state == STATE_ACTIVE) {
			results[id] = result + contribution;
		}
	}
}