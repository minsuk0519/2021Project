#version 430 core
layout (location = 0) in vec3 inPos;

out vec3 TexCoord;

uniform mat4 projection;
uniform mat3 mat3view;

void main()
{
	gl_Position = (projection * mat4(mat3view)	* vec4(inPos, 1.0)).xyww;
	
	TexCoord = inPos;
}