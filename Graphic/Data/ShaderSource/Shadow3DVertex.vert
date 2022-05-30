#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Fragpos;
out vec3 Normal;
out vec4 lightSpacePos;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

layout (location = 102) uniform mat4 lightMat;

void main()
{
    Fragpos = (model * vec4(aPos, 1.0)).xyz;
	gl_Position = projection * view * vec4(Fragpos, 1.0);
	//Normal = transpose(inverse(mat3(view * model))) * aNormal;
	Normal = transpose(inverse(mat3(model))) * aNormal;
	lightSpacePos = lightMat * vec4(Fragpos, 1.0);
	Fragpos = (vec4(Fragpos, 1.0)).xyz;
}