#version 430 core
out vec4 FragColor;

in vec3 TexCoord;

uniform samplerCube boxTex;

void main()
{
	FragColor = vec4(texture(boxTex, TexCoord));
}