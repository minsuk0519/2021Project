#version 430 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 lightColor;
  
in vec2 TexCoord;

uniform sampler2D posTex;
uniform sampler2D normalTex;
uniform sampler2D colTex;
uniform sampler2D specTex;
uniform sampler2D aoTex;

struct Light {
	vec3 Position;
	vec3 Color;
	
	float Linear;
	float Quadratic;
	
	samplerCube depthCubeMap;
};

struct Flashlight 
{	
	vec3 Color;
	
	float linear;
	float quadratic;
	
	float cutOff;
	float outercutOff;
};

#define MAX_LIGHT 5

uniform Light light[MAX_LIGHT];

uniform vec3 viewPos;

uniform sampler2D shadowTex;

uniform int light_num;

uniform bool spotlight;
uniform Flashlight flash;
uniform vec3 viewdir;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);


vec3 CalcLight(Light light, vec3 fragpos, vec3 norm, vec3 viewDir, vec3 Diffuse, float spec, float ao)
{
	vec3 fragToLight = fragpos - light.Position;

	float distance = length(fragToLight);
	float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * (distance * distance));
		
	vec3 ambientcolor = light.Color * Diffuse * ao;
			
	//diffuse
	vec3 lightDir = normalize(-fragToLight);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffusecolor = light.Color * diff * Diffuse;
						
	//specular
	vec3 halfwayDir = normalize(lightDir - viewDir);
	float Spec = pow(max(dot(norm, halfwayDir), 0.0), 128.0);
	vec3 specularcolor = light.Color * Spec * spec;

	float currentDepth = length(fragToLight);
	
	float shadow = 0.0;
	float bias = 0.1;
	float diskRadius = (1.0 + length(viewPos - fragpos) / 100.0) / 25.0;
	for(int i = 0; i < 20; ++i)
	{	
		float closestDepth = texture(light.depthCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= 100.0;
		if(currentDepth - bias > closestDepth)
		{
			shadow += 1.0;
		}
	}
	shadow /= 20.0;
	shadow = 1.0 - shadow;
	
	float specshadow = shadow;
	if(shadow < 0.5) specshadow = 0.0;
		
	vec3 result = ((ao * (0.05 + shadow) * 0.5 + diff * shadow * 0.8) * Diffuse + Spec * spec * specshadow * 0.7) * light.Color * attenuation;

	return result;
}

vec3 CalcFlashlight(Flashlight flash, vec3 fragpos, vec3 norm, vec3 viewDir, vec3 Texture, float spec)
{
	vec3 lightDir = normalize(viewPos - fragpos);
	
	//diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffusecolor = flash.Color * diff * Texture;
		
	//specular
	vec3 halfDir = normalize(lightDir - viewDir);
	float Spec = pow(max(dot(norm, halfDir), 0.0), 128.0);
	vec3 specularcolor = flash.Color * Spec * spec;
		
	//spotlight
	vec3 flashDir = normalize(viewPos - fragpos);
	float theta = dot(flashDir, normalize(-viewdir));
	float epsilon = (flash.cutOff - flash.outercutOff);
	float intensity = clamp((theta - flash.outercutOff) / epsilon, 0.0, 1.0);
		
	//attenuation
	float distance = length(viewPos - fragpos);
	float attenuation = 1.0 / (1.0 + flash.linear * distance + flash.quadratic * (distance * distance));
	
	diffusecolor *= attenuation * intensity * 5.0;
	specularcolor *= attenuation * intensity * 5.0;
	
	return diffusecolor + specularcolor;
}

void main()
{
    vec3 FragPos = texture(posTex, TexCoord).xyz;
    vec3 Normal = texture(normalTex, TexCoord).xyz;	
    vec3 Diffuse = texture(colTex, TexCoord).rgb;

    float Specular = texture(specTex, TexCoord).r;
	vec3 viewDir = normalize(FragPos - viewPos);
	float ao = texture(aoTex, TexCoord).r;
	
	vec3 result = vec3(0,0,0);

	if(length(Normal) < 0.1)
	{
		result = Diffuse;
	}
	else
	{
		for(int i = 0; i < light_num; ++i)
		{
			result += CalcLight(light[i], FragPos, Normal, viewDir, Diffuse, Specular, ao);
		}
		if(spotlight)
		{
			result += CalcFlashlight(flash, FragPos, Normal, viewDir, Diffuse, Specular);
		}
	}
	
	float brightness = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness >= 0.9)
        lightColor = vec4(result.rgb, 1.0);
    else
        lightColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	FragColor = vec4(result, 1.0);
}