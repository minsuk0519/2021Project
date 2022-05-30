#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aRand;

out vec2 TexCoord;
out vec3 Fragpos;
out vec3 Normal;

layout (location = 100) uniform mat4 model;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

uniform float time;

mat4 rotation3d(vec3 axis, float angle) 
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return mat4(
		oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
		oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
		0.0,                                0.0,                                0.0,                                1.0
	);
}

void main()
{
	float A = gl_InstanceID + time;
	
	float T = A * 60 / 600.0;
	
	mat4 rot = rotation3d(aRand, (aRand.z + 0.5) * time * 0.1f);
	Fragpos = (rot * vec4(aPos, 1.0)).xyz;
	
    Fragpos = (model * vec4(Fragpos, 1.0)).xyz;
	
	
	
	bool flip = false;
	
	while(T > 60.0) T -= 60.0;
	
	if(T > 30)
	{
		T = 60 - T;
		flip = true;
	}
	
	float x = -0.04 * (T * T - 30 * T) * sin(T);
	float y = T * 2 / 3;
	float z = -0.04 * (T * T - 30 * T) * cos(T);
	
	if(flip) Fragpos += vec3(z, y, x);
	else Fragpos += vec3(x, y, z);
	
	
	gl_Position = projection * view * vec4(Fragpos, 1.0);
	Normal = transpose(inverse(mat3(rot * model))) * aNormal;
	
	TexCoord = aTexCoord;
}