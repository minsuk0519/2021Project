#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VERT_OUT {
	vec3 Fragpos;
	vec3 Normal;
} fragnorm;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

void main()
{
	vec3 fragpos = (model * vec4(aPos, 1.0)).xyz;
    fragnorm.Fragpos = fragpos;
	gl_Position = projection * view * vec4(fragpos, 1.0);
	fragnorm.Normal = transpose(inverse(mat3(model))) * aNormal;
}