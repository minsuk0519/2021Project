#version 430 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D texID;

uniform bool horizontal;

void main()
{
	float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

    vec2 tex_offset = 1.0 / textureSize(texID, 0);
    vec3 result = texture(texID, TexCoord).rgb * weight[0];
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
			vec2 newTex = TexCoord + vec2(tex_offset.x * i, 0.0);
			if(newTex.y > 1.0) newTex.x = 1.0;
			result += texture(texID, newTex).rgb * weight[i];
			
			newTex = TexCoord - vec2(tex_offset.x * i, 0.0);
			if(newTex.x < 0.0) newTex.x = 0.0;
			result += texture(texID, newTex).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
			vec2 newTex = TexCoord + vec2(0.0, tex_offset.y * i);
			if(newTex.y > 1.0) newTex.y = 1.0;
			result += texture(texID, newTex).rgb * weight[i];
			
			newTex = TexCoord - vec2(0.0, tex_offset.y * i);
			if(newTex.y < 0.0) newTex.y = 0.0;
			result += texture(texID, newTex).rgb * weight[i];
		}
	}
    FragColor = vec4(result, 1.0);
    FragColor = vec4(texture(texID, TexCoord).rgb, 1.0);
}