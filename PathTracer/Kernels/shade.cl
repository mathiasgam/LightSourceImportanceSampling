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
		printf("ERROR: index: %d, num_lights: %d, r: %f\n", index, num_lights, r);
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
	const float pdf = cdf[index + num_lights];
	/*
	if (pdf < 0.0f || pdf > 1.0f) {
		printf("PDF: %f, index: %d, num_lights: %d, cdf[i]: %f, cdf[i+1]: %f\n", pdf, index, num_lights, cdf[index], cdf[index + 1]);
	}
	*/
	//*pdf = 1.0f; // pdf is canceling out with power / area calculation, so all lights uses their material emission
	// only works for area lights. need to calculate per light pdf if both point and area lights are used
	*pdf_out = pdf;
	return index;
}

float sqr(float x) {
	return x * x;
}

float triangle_solid_angle(float3 shading_point, float3 p0, float3 p1, float3 p2) {
	const float3 r0 = normalize(p0 - shading_point);
	const float3 r1 = normalize(p1 - shading_point);
	const float3 r2 = normalize(p2 - shading_point);

	const float N = length(dot(r0, cross(r1, r2)));
	const float D = 1.0f + dot(r0, r1) + dot(r0, r2) + dot(r1, r2);

	return 2.0f * atan2(N, D);
}

inline float3 sample_triangle(float3 ab, float3 ac, float r1, float r2) {
	if (r1 + r2 > 1.0f) {
		r1 = 1.0f - r1;
		r2 = 1.0f - r2;
	}
	return ab * r1 + ac * r2;
}

const sampler_t sampler_in = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;

// Only handle diffuse lighting
float3 BRDF(float3 w_in, float3 w_out, float3 diffuse, float3 normal) {
	const float3 cos_theta_in = max(-dot(w_in, normal), 0.0f);
	const float3 cos_theta_out = max(dot(w_out, normal), 0.0f);

	return diffuse * cos_theta_in * cos_theta_out;
}

// Brute force finding the angle that captures the entire box
float max_angle(float3 pmin, float3 pmax, float3 position, float3 normal) {
	//if (contained(pmin, pmax, position)) // if inside the bounding box, a light can be in all directions
	//	return M_PI_F;
	const float3 center = (pmin + pmax) * 0.5f;
	const float3 diff = center - position;
	const float3 dir = normalize(diff);

	// calculate the points from the center to each corner
	float3 points[8];
	points[0] = normalize((float3)(pmin.x, pmin.y, pmin.z) - position);
	points[1] = normalize((float3)(pmax.x, pmin.y, pmin.z) - position);
	points[2] = normalize((float3)(pmin.x, pmax.y, pmin.z) - position);
	points[3] = normalize((float3)(pmax.x, pmax.y, pmin.z) - position);

	points[4] = normalize((float3)(pmin.x, pmin.y, pmax.z) - position);
	points[5] = normalize((float3)(pmax.x, pmin.y, pmax.z) - position);
	points[6] = normalize((float3)(pmin.x, pmax.y, pmax.z) - position);
	points[7] = normalize((float3)(pmax.x, pmax.y, pmax.z) - position);

	// Project the points from each corner onto the plane defined by tangent and bitangent and find their sqr length
	float min_cos_theta = 1.0f;
	for (int i = 0; i < 8; i++) {
		const float cos_theta = dot(dir, points[i]);
		min_cos_theta = min(min_cos_theta, cos_theta);
	}
	// the longest sqr length is used to find the maximum angle. only do the expensive calculation once
	return acos(min_cos_theta);;
}

inline float importance(LightTreeNode node, float3 position, float3 normal, float3 diffuse) {
	const float3 center = (node.pmin.xyz + node.pmax.xyz) * 0.5f;
	const float3 diff = center - position;
	const double dist = length(diff);
	const double sqr_dist = sqr(dist);
	const float3 dir = normalize(diff);

	const float theta = acos(-dot(node.axis.xyz, dir));
	const float theta_i = acos(dot(normal, dir));

	// atan of radius of bounding sphere divided by distance
	const float theta_u = max_angle(node.pmin.xyz, node.pmax.xyz, position, normal);

	const float theta_t = max(0.0f, (theta - THETA_O(node)) - theta_u);
	const float theta_ti = max(0.0f, theta_i - theta_u);

	if (theta_t < THETA_E(node))
		return 0.0f;

	const float3 I = (diffuse * fabs(cos(theta_ti)) * node.energy.xyz) * inverse(sqr_dist) * cos(theta_t);
	//const float3 I = (diffuse * node.energy.xyz) * inverse(sqr_dist);

	return I.x + I.y + I.z;
}

inline int pick_light(__global const LightTreeNode* nodes, float3 position, float3 normal, float3 diffuse, double r, float* pdf_out) {
	LightTreeNode node = nodes[0];
	double pdf = 1.0f;
	double xi = r;
	while (true) {
		if (LEAF(node)) { // node is leaf
			float estimations[10];

			// Fetch range of ids
			const int start_index = INDEX(node);
			const int count = COUNT(node);

			// construct cdf
			float3 sum = (float3)(0.0f);
			for (int i = 0; i < count; i++) {

			}

			// find light by xi and predecessor search

			
			pdf *= inverse((float)(count));

			// return values
			*pdf_out = pdf;
			return INDEX(node);
		}
		else { // node is internal
			const double I_l = importance(nodes[node.left], position, normal, diffuse);
			const double I_r = importance(nodes[node.right], position, normal, diffuse);

			const double sum = I_l + I_r;

			const double p_l = sum == 0.0 ? 0.5 : I_l / sum;
			const double p_r = sum == 0.0 ? 0.5 : I_r / sum;
			/*
			if (get_global_id(0) == 0)
				printf("I_l: %f, I_r: %f, sum: %f, p_l: %f, p_r: %f\n", I_l, I_r, sum, p_l, p_r);
			*/

			if (xi < p_l) {
				xi = xi / p_l;
				node = nodes[node.left];
				pdf *= p_l;
			}
			else {
				xi = (xi - p_l) / p_r;
				node = nodes[node.right];
				pdf *= p_r;
			}

		}
	}
}

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
#ifdef USE_LIGHTTREE
	IN_BUF(LightTreeNode, light_tree_nodes),
#else
	IN_BUF(float, light_power_cdf),
#endif
	OUT_BUF(float3, results),
	OUT_BUF(float3, throughputs),
	OUT_BUF(int, states),
	OUT_BUF(float3, light_contribution),
	OUT_BUF(Ray, bounce_rays),
	OUT_BUF(Ray, shadow_rays),
	__read_only image2d_t texture
) {
	int id = get_global_id(0);
	uint rng = hash2(hash1(id) ^ hash1(seed));

	//barrier(CLK_GLOBAL_MEM_FENCE);

	if (id < num_samples) {
		GeometricInfo geometric = geometrics[id];
		Intersection hit = hits[id];

		int state = states[id];

		// iterate over all lights
		//for (uint i = 0; i < num_lights; i++){
		if (state & STATE_ACTIVE) {

			float3 result = results[id];
			float3 throughput = throughputs[id];


			// process miss
			if (hit.hit == 0) {
				float2 coord = direction_to_hdri(geometric.incoming.xyz);
				result += read_imagef(texture, sampler_in, coord).xyz * throughput;
				//result += throughput;
				state = STATE_INACTIVE;
			}
			else {
				const Material material = materials[hit.material_index];
				const float3 diffuse = material.diffuse.xyz;
				const float3 emission = material.emission.xyz;

				// Handle emissive hit
				if (state & STATE_FIRST) {
					// first sample does not have the next event estimation
					result += emission * throughput;
					state = STATE_ACTIVE;
				}
				else {
					// Next event estimation is also a sample, 
					// so the average of next event and emissive hit is used to combine the two into one
					result += emission * throughput * 0.0f;
				}


				if (any(isgreater(emission, 0.0f))) {
					state = STATE_INACTIVE;
					throughput = (float3)(0.0f);
				}
				else
				{



					throughput *= diffuse / M_PI_F;

					const float3 position = geometric.position.xyz;
					const float3 normal = geometric.normal.xyz;

#ifdef USE_LIGHTTREE
					// choose light
					//uint i = random_uint(&rng, num_lights);
					//float pdf = inverse(num_lights);
					double r = random_double(&rng);
					float pdf;
					uint i = pick_light(light_tree_nodes, position, normal, diffuse, r, &pdf);
#else
					float r = rand(&rng);
					float pdf;
					uint i = select_light(light_power_cdf, num_lights, r, &pdf);
#endif

					// Handle direct light
					Light light = lights[i];
					float3 light_pos = light.position.xyz;
					const float area = length(cross(light.tangent.xyz, light.bitangent.xyz)) * 0.5f;

					if (isnan(area)) {
						printf("ERROR: area is nan!\n");
					}

					if (light.position.w == 1.0f) { // if light is triangle, then sample the position
						const float r1 = rand(&rng);
						const float r2 = rand(&rng);
						light_pos += sample_triangle(light.tangent.xyz, light.bitangent.xyz, r1, r2);
						// PDF has precalculated 1.0/area for triangle lights
#ifdef USE_LIGHTTREE
						pdf *= inverse(area);
#endif
					}

					const float3 diff = light_pos - position;
					const float dist = length(diff);
					const float dist_inv = inverse(dist);
					const float3 dir = diff * dist_inv;

					const float cos_theta = max(dot(geometric.normal.xyz, dir), 0.0f);
					const float cos_theta_light = max(-dot(light.direction.xyz, dir), 0.0f);

					// calculate the lights contribution
					const float3 intensity = light.intensity.xyz;
					const float FTR = inverse(area); // lamberts Five Times Rule. only works if applied all the time...
					//const float FTR = 1.0f;

#ifdef SOLID_ANGLE
					const float Omega = triangle_solid_angle(position, light.position.xyz, light.position.xyz + light.tangent.xyz, light.position.xyz + light.bitangent.xyz);
#else
					const float Omega = cos_theta_light * area * inv_sqr(dist);
#endif
					const float3 L_i = intensity * cos_theta * Omega * ceil(cos_theta_light) * inverse(area); // WHY !?!?

					// lift shading point to avoid hitting the geometry again
					const float3 lift = geometric.normal.xyz * 10e-6f;

					if (any(isnan(L_i))) {
						printf("Error: L_i has nan value: [%f,%f,%f]\n", L_i.x, L_i.y, L_i.z);
					}

					if (isnan(pdf)) {
						printf("Error: pdf is nan\n");
					}

					if (any(isnan(throughput))) {
						printf("Error: throughput has nan value\n");
					}

					shadow_rays[id] = CreateRay(geometric.position.xyz + lift, dir, 0.0f, dist - 10e-5f);
					const float3 L = throughput * L_i * inverse(pdf) * 1.0f;
					light_contribution[id] = L;

#ifdef RUSSIAN_ROULETTE
					// Russian roulette
					float pdf_russian = (throughput.x + throughput.y + throughput.z) / 3.0f;
					if (rand(&rng) > pdf_russian) {
						state = STATE_INACTIVE;
					}
					else {
						throughput = throughput * inverse(pdf_russian);
					}
#endif

					float3 out_dir = sample_hemisphere_cosine(&rng, geometric.normal.xyz);
					const float cos_theta_out = max(dot(geometric.normal.xyz, out_dir), 0.0f);
					//const float pdf_bounce = 1.0f / (2.0f * M_PI_F); // uniform sampling
					const float pdf_bounce = 1.0f / M_PI_F; // cosine sampling cos_theta / pi, but cos_theta cancels out with cosine sampling

					bounce_rays[id] = CreateRay(geometric.position.xyz + lift, out_dir, 0.0f, 1000.0f);

					//pdf_bounce *= 1.0f / M_PI_F;

					throughput *= inverse(pdf_bounce);

					// should not be nessesary as cosine hemisphere sampling cancels out;
					//throughput *= max(dot(geometric.normal.xyz, out_dir), 0.0f);
				}
			}

			states[id] = state;
			results[id] = result;
			throughputs[id] = throughput;
		}

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