#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 3) in vec3 aOffset;

out vec2 TexCoord;
out vec3 Fragpos;
out vec3 Normal;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

void main()
{
    vec3 axis = normalize(aOffset);
    float cx = cos(aOffset.z * 1.5);
    float sx = sin(aOffset.z * 1.5);
    float cy = cos(aOffset.z * 1.5);
    float sy = sin(aOffset.z * 1.5);
    float cz = cos(aOffset.x * 1.5);
    float sz = sin(aOffset.x * 1.5);
    
    mat4 M = mat4(	cz*cy, cz*sy*sx+sz*cx, -cz*sy*cx+sz*sx, 0,
					-sz*cy, -sz*sy*sx+cz*cx, sz*sy*cx+cz*sx, 0,
					sy, -cy*sx, cy*cx, 0,
					0, 0, 0, 1);
					
					
	vec3 norm = normalize(aOffset);
	vec3 up = normalize(vec3(0, 1, 0));
	vec3 c = cross(norm, up);
	mat4 A = mat4(c.x, up.x, norm.x, 0.0,
					c.y, up.y, norm.y, 0.0,
					c.z, up.z, norm.z, 0.0,
					0.0, 0.0, 0.0, 1.0);
	 
	 
	Fragpos = (model * A * vec4(aPos, 1.0)).xyz + aOffset;
	gl_Position = projection * view * vec4(Fragpos, 1.0);
	Normal = transpose(inverse(mat3(view * model))) * aNormal;
	
	TexCoord = aTexCoord;
}