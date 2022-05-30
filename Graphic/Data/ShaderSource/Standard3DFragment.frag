#version 430 core
layout (location = 0) out vec4 FragColor;

layout (location = 1) out vec4 positionColor;
layout (location = 2) out vec4 normalColor;
layout (location = 3) out vec4 albedoColor;
layout (location = 4) out vec4 specularColor;


uniform vec3 objectcolor;

void main()
{
	float brightness = dot(objectcolor, vec3(0.7,0.7,0.7));
	if(brightness > 1.0)
	{
		//lightColor = vec4(objectcolor, 1.0);
	}
	else
	{
		//lightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	positionColor = vec4(0,0,0,1);
	normalColor = vec4(0,0,0,1);
	albedoColor = vec4(objectcolor, 1.0);
	specularColor = vec4(1,1,1,1);

	FragColor = vec4(objectcolor, 1.0f);
}