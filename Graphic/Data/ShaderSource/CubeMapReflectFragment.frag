#version 430 core
out vec4 FragColor;
  
in vec3 Normal;
in vec3 Fragpos;

layout (location = 101) uniform vec3 viewPos;
uniform samplerCube cubeMap;

void main()
{
	//vec3 ref = reflect(normalize(Fragpos - viewPos), normalize(Normal));
	vec3 ref = refract(normalize(Fragpos - viewPos), normalize(Normal), 1.00 / 2.42);

    FragColor = vec4(texture(cubeMap, ref).rgb, 1.0);
}