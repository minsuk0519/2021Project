#version 430 core
layout (location = 0) out vec4 FragColor;
  
layout (location = 1) out vec4 positionColor;
layout (location = 2) out vec4 normalColor;
layout (location = 3) out vec4 albedoColor;
layout (location = 4) out vec4 specularColor;  
  
in vec3 Fragpos;
in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in mat3 Model;

uniform vec2 scale2d;

uniform sampler2D Texture;
uniform sampler2D normalTex;
uniform sampler2D parallaxTex;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 128.0;
	float layerNum = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
	
  	float layerHeight = 1.0 / layerNum;
	float currentLayer = 0.0;
	
	vec2 parallax = viewDir.xy * 0.05;
	vec2 deltaTexCoord = parallax / layerNum;
	
	vec2 currentTex = texCoords;
	float currentHeight = texture(parallaxTex, currentTex).r;
	
	while(currentLayer < currentHeight)
	{
		currentTex -= deltaTexCoord;
		currentHeight = texture(parallaxTex, currentTex).r;
		currentLayer += layerHeight;
	}
	
	vec2 prevTex = currentTex + deltaTexCoord;
	
	float afterHeight = currentHeight - currentLayer;
	float beforeHeight = texture(parallaxTex, prevTex).r - currentLayer + layerHeight;
	
	float weight = afterHeight / (afterHeight - beforeHeight);
	vec2 result = prevTex * weight + currentTex * (1.0 - weight);
	
	return result;
}

void main()
{
	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

	vec2 A = TexCoord.xy;
	A *= scale2d / 5;

	vec2 tex = ParallaxMapping(A, viewDir);
	
	//if(tex.x > 1.0 || tex.y > 1.0 || tex.x < 0.0 || tex.y < 0.0)
		//discard;
	
	vec3 normal = texture(normalTex, tex).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	vec3 n = transpose(inverse(Model)) * normal;
	normalColor = vec4(normalize(n), 1.0);
	
	vec3 color = texture(Texture, tex).rgb;
	
	vec3 ambient = 0.2 * color;
	vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
	
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * color;
	
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	
	vec3 specular = vec3(0.2) * spec;
	FragColor = vec4(color, 1.0);//vec4(ambient + diffuse + specular, 1.0);
	
	positionColor = vec4(Fragpos, 1.0);
	albedoColor = vec4(diffuse + ambient, 1.0);
	specularColor = vec4(spec, spec, spec, 1.0);
}