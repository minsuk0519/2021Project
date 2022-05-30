#version 430 core
layout (location = 0) out vec4 FragColor;
//layout (location = 1) out vec4 lightColor;

layout (location = 1) out vec4 positionColor;
layout (location = 2) out vec4 normalColor;
layout (location = 3) out vec4 albedoColor;
layout (location = 4) out vec4 specularColor;

  
in vec3 Normal;
in vec3 Fragpos;
in vec4 lightSpacePos;

struct Material 
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
	float shininess;
};

struct Light
{
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	float constant;
	float linear;
	float quadratic;
};

struct Flashlight
{
	Light light;
	
	vec3 direction;
	float cutOff;
	float outercutOff;
};

#define LIGHT_MAX_NUM 4

layout (location = 101) uniform vec3 viewPos;

uniform Material material;
uniform Light light[LIGHT_MAX_NUM];
uniform Flashlight flash;
uniform vec3 sundir;

uniform bool isflash;
uniform int lightnum;

uniform sampler2D shadowTex;
uniform samplerCube depthCube[LIGHT_MAX_NUM];

uniform float far_plane;

float CalcShadow(vec4 pos, float bias)
{
	vec3 coord = pos.xyz / pos.w;
	coord = coord * 0.5 + 0.5;
	
	float closestDepth = texture(shadowTex, coord.xy).r;
	
	float currentDepth = coord.z;
	
	if(coord.z > 1.0) return 0.0;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowTex, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float neardepth = texture(shadowTex, coord.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > neardepth ? 1.0 : 0.0;        
		}    
	}
	
    return shadow / 9.0;
}

vec3 CalcLight(Light light, vec3 norm, vec3 viewDir, int index)
{
	vec3 ambientcolor = light.ambient * material.ambient;
	
	vec3 lightDir = normalize(light.position - Fragpos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffusecolor = light.diffuse * diff * material.diffuse;
	
	//specular
	vec3 reflectDir = reflect(-lightDir, norm);
	float Spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0);
	vec3 specularcolor = light.specular * Spec * material.specular;
	
	float distance = length(light.position - Fragpos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	

	vec3 fragToLight = Fragpos - light.position;
    float currentDepth = length(fragToLight);
	
	vec3 sampleOffsetDirections[20] = vec3[]
	(
	   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);
	float shadow = 0.0;
	float viewDistance = length(viewPos - Fragpos);
	float bias = 0.001;
	float diskRadius = 0.05;
	for(int i = 0; i < 20; ++i)
	{
		float closestDepth = texture(depthCube[index], fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= 100.0;
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= 20.0; 

	shadow = 1.0 - shadow;
		
	ambientcolor *= attenuation;
	diffusecolor *= attenuation * shadow;
	specularcolor *= attenuation * shadow;

	vec3 result = ambientcolor + diffusecolor + specularcolor;	

	return result;
}

vec3 CalcFlashlight(Flashlight flash, vec3 norm, vec3 viewDir)
{
	Light light = flash.light;
	vec3 lightDir = normalize(light.position - Fragpos);
	
	float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffusecolor = light.diffuse * 2.0 * diff * material.diffuse;
	vec3 diffusecolor = vec3(1,1,1) * 2.0 * diff * material.diffuse;
	
	vec3 reflectDir = reflect(-lightDir, norm);
	float Spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0);
	vec3 specularcolor = vec3(1,1,1) * Spec * material.specular;
	//vec3 specularcolor = light.specular * Spec * material.specular;
	
	vec3 flashDir = normalize(flash.light.position - Fragpos);
	float theta = dot(flashDir, normalize(-flash.direction));
	float epsilon = (flash.cutOff - flash.outercutOff);
	float intensity = clamp((theta - flash.outercutOff) / epsilon, 0.0, 1.0);
	
	float distance = length(flash.light.position - Fragpos);
	float attenuation = 1.0 / (flash.light.constant + flash.light.linear * distance + flash.light.quadratic * (distance * distance));
	
	diffusecolor *= attenuation * intensity;
	specularcolor *= attenuation * intensity;
	
	return diffusecolor + specularcolor;
}

vec3 CalcDirLight(vec3 direction, vec3 norm, vec3 viewDir)
{
	vec3 ambientcolor = vec3(0.2, 0.2, 0.2) * material.ambient;
	
	vec3 lightDir = normalize(-direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffusecolor = vec3(0.5, 0.5, 0.5) * diff * material.diffuse;
	
	vec3 reflectDir = reflect(-lightDir, norm);
	float Spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0);
	vec3 specularcolor = vec3(1.0, 1.0, 1.0) * Spec * material.specular;
	
	float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);

	float shadow = 1.0 - CalcShadow(lightSpacePos, bias);

	diffusecolor *= shadow;
	specularcolor *= shadow;

	vec3 result = ambientcolor + diffusecolor + specularcolor;	
	
	return ambientcolor + diffusecolor + specularcolor;
}

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - Fragpos);

	vec3 result;
	for(int i = 0; i < lightnum; ++i)
	{
		//result += CalcLight(light[i], norm, viewDir, i);
	}
	if(isflash)
	{
		//result += CalcFlashlight(flash, norm, viewDir);
	}
	//result += CalcDirLight(sundir, norm, viewDir);
	


	if(true)
	{
		//result = pow(result, vec3(1.0/1.6));
	}
	
	float brightness = dot(result, vec3(0.7,0.7,0.7));
	if(brightness > 1.0)
	{
		//lightColor = vec4(result, 1.0);
	}
	else
	{
		//lightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	
	positionColor = vec4(Fragpos, 1.0);
	normalColor = vec4(norm, 1.0);
	albedoColor = vec4(material.ambient, 1.0);
	specularColor = vec4(material.specular, 1.0);
	
	

    //FragColor = vec4(result, 1.0);
}