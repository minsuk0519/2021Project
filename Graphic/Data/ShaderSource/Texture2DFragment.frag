#version 430 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D texID;
uniform sampler2D blurTex;

void main()
{
	vec3 temp[9];
	vec3 col = vec3(0.0);
	float degree = 100.0;
	
	for(int i = -1; i < 2; ++i)
	{
		for(int j = -1; j < 2; ++j)
		{
			vec2 offset = vec2(j / degree, i / degree);
			if(i == 1 && j == 1)
			{
				//col += 0.25 * texture(texID, TexCoord + offset).rgb;
			}
			else if((i + j) % 2 == 0)
			{
				//col += 0.0625 * texture(texID, TexCoord + offset).rgb;
			}
			else
			{
				//col += 0.125 * texture(texID, TexCoord + offset).rgb;
			}
		}
	}
	
	float kernel[9] = float[](
		//-1, -1, -1, -1, 9, -1, -1, -1, -1
		0.0625, 0.125, 0.0625,
		0.125, 0.25, 0.125,
		0.0625, 0.125, 0.0625
	);
	
	for(int i = 0; i < 9; ++i)
	{
		//col += temp[i] * kernel[i];
	}
	
	col = texture(texID, TexCoord).rgb;
	col += texture(blurTex, TexCoord).rgb;
		FragColor = vec4(col, 1.0);
return;
	
	vec3 result = vec3(1.0) - exp(-col * 0.5);
	result = pow(result, vec3(1.0 / 2.2));
	FragColor = vec4(result, 1.0);
		
	FragColor = vec4(col / 3.0, 1.0);
}