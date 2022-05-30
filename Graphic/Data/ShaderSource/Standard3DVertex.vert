#version 430 core
layout (location = 0) in vec3 aPos;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}