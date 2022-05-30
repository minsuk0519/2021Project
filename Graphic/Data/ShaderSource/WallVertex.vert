#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out vec3 Fragpos;
out vec2 TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out mat3 Model;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

layout (location = 101) uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
	Fragpos = vec3(model * vec4(aPos, 1.0));
	TexCoord = aTexCoord;
	
	mat3 normalMatrix = mat3(model);
	//mat3 normalMatrix = transpose(inverse(mat3(model)));
	Model = mat3(model);
	
	vec3 Tan = normalize(normalMatrix * aTangent);
	vec3 Norm = normalize(normalMatrix * -aNormal);
	Tan = normalize(Tan - dot(Tan, Norm) * Norm);
	vec3 BiTan = normalize(normalMatrix * aBiTangent);
	//vec3 BiTan = cross(Norm, Tan);
	
	mat3 TBN = transpose(mat3(Tan, BiTan, Norm));
	TangentLightPos = TBN * lightPos;
	TangentViewPos = TBN * viewPos;
	TangentFragPos = TBN * Fragpos;
	Fragpos = vec3(vec4(Fragpos, 1.0));
	
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}