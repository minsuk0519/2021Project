#version 430 core
out vec4 FragColor;
  
in vec3 Normal;
in vec3 Fragpos;
  
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

vec3 CalcLight(Light light, vec3 norm, vec3 viewDir)
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
	
	ambientcolor *= attenuation;
	diffusecolor *= attenuation;
	specularcolor *= attenuation;

	vec3 result = ambientcolor + diffusecolor + specularcolor;
	
	return result;
}

vec3 CalcFlashlight(Flashlight flash, vec3 norm, vec3 viewDir)
{
	Light light = flash.light;
	vec3 lightDir = normalize(light.position - Fragpos);
	
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffusecolor = light.diffuse * 2.0 * diff * material.diffuse;
	
	vec3 reflectDir = reflect(-lightDir, norm);
	float Spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128.0);
	vec3 specularcolor = light.specular * Spec * material.specular;
	
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
	
	return ambientcolor + diffusecolor + specularcolor;
}

void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - Fragpos);

	vec3 result;
	for(int i = 0; i < lightnum; ++i)
	{
		result += CalcLight(light[i], norm, viewDir);
	}
	if(isflash)
	{
		result += CalcFlashlight(flash, norm, viewDir);
	}
	result += CalcDirLight(sundir, norm, viewDir);
	
    FragColor = vec4(result, 1.0);
}