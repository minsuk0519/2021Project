#version 430 core

in vec4 FragPos;

in float AA;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	float distance = length(FragPos.xyz - lightPos);
	
	distance /= far_plane;

	gl_FragDepth = distance;
}