#version 330 core

in vec3 v_normal;

out vec4 FragColor;

uniform vec4 color;

void main()
{
	vec3 light_dir = normalize(vec3(-1,-1,-1));
	float d = -dot(v_normal, light_dir);
	float ambient = 0.2;

	vec3 norm_color = (v_normal * 0.5) + 0.5;
	FragColor = color * max(ambient, d);
	//FragColor = vec4(norm_color, 1.0);
};