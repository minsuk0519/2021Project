#version 430 core
layout (location = 1) out vec4 positionColor;
layout (location = 2) out vec4 normalColor;
layout (location = 3) out vec4 albedoColor;
layout (location = 4) out vec4 specularColor;
  
in vec2 TexCoord;
in vec3 Normal;
in vec3 Fragpos;

struct Material
{
	sampler2D texture;
	sampler2D texture2;
	float shininess;
};

layout (location = 101) uniform vec3 viewPos;

uniform Material material;

void main()
{
	positionColor = vec4(Fragpos, 1.0);
	normalColor = vec4(normalize(Normal), 1.0);
	albedoColor = texture(material.texture, TexCoord);
	albedoColor = albedoColor * albedoColor;
	specularColor = texture(material.texture2, TexCoord);
}