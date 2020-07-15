#version 330 core

in vec3 v_normal;

out vec4 FragColor;

uniform vec4 color;

void main()
{
	//FragColor = color;
	vec3 norm_color = (v_normal * 0.5) + 0.5;
	FragColor = vec4(norm_color, 1.0);
};