#version 330 core
		
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 model;
uniform mat4 cam_matrix;

out vec3 v_normal;

void main()
{
	v_normal = (model * vec4(normal, 0.0)).xyz;
	gl_Position = cam_matrix * model * vec4(position.xyz, 1.0);
};