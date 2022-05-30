#version 430 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D aoTex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(aoTex, 0));
    float result = 0.0;
    for (int x = -1; x < 2; ++x) 
    {
        for (int y = -1; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(aoTex, TexCoord + offset).r;
        }
    }
    
	result = result / (3.0 * 3.0);

	FragColor = vec4(result, result, result, 1.0);
}