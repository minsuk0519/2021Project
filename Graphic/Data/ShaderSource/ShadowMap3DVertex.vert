#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (location = 100) uniform mat4 model;
layout (location = 102) uniform mat4 lightMat;

void main()
{
	vec3 Normal = transpose(inverse(mat3(model))) * aNormal;

	gl_Position = lightMat * model * vec4(aPos, 1.0);
}