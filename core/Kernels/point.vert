#version 330 core
		
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 v_color;

uniform mat4 cam_matrix;

void main()
{
	v_color = color;
	gl_Position = cam_matrix * vec4(position.xyz, 1.0);
};