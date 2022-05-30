#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VERT_OUT {
	vec2 TexCoord;
	vec3 Fragpos;
	vec3 Normal;
	vec3 ViewNormal;
	mat4 view;
} vert_out;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

void main()
{
    vert_out.Fragpos = (model * vec4(aPos, 1.0)).xyz;
	gl_Position = projection * view * vec4(vert_out.Fragpos, 1.0);
	vert_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vert_out.ViewNormal = transpose(inverse(mat3(view * model))) * aNormal;
	
	vert_out.TexCoord = aTexCoord;
	
	vert_out.view = projection * view;
}