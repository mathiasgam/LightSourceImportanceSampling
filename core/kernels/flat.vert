#version 330 core
		
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 cam_matrix;

void main()
{
	gl_Position = cam_matrix * model * vec4(pos.x, pos.y, pos.z, 1.0);
};