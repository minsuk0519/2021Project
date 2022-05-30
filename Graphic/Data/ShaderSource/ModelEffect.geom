#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VERT_OUT {
	vec2 TexCoord;
	vec3 Fragpos;
	vec3 Normal;
	vec3 ViewNormal;
	mat4 view;
} vert_out[];

out vec3 Fragpos;
out vec3 Normal;
out vec2 TexCoord;

uniform float time;

void main()
{
	float T = time * 0.1;
	if(T > 5)
	{
		T = 5;
	}

	Fragpos = vert_out[0].Fragpos;
	Normal = vert_out[0].Normal;
	TexCoord = vert_out[0].TexCoord;
	gl_Position = gl_in[0].gl_Position + vert_out[0].view * vec4(Normal,0) * 3.0 * (1 / T - 0.2);
	EmitVertex();
	
	Fragpos = vert_out[1].Fragpos;
	Normal = vert_out[1].Normal;
	TexCoord = vert_out[1].TexCoord;
	gl_Position = gl_in[1].gl_Position + vert_out[0].view * vec4(Normal,0) * 3.0 * (1 / T - 0.2);
	EmitVertex();
	
	Fragpos = vert_out[2].Fragpos;
	Normal = vert_out[2].Normal;
	TexCoord = vert_out[2].TexCoord;
	gl_Position = gl_in[2].gl_Position + vert_out[0].view * vec4(Normal,0) * 3.0 * (1 / T - 0.2);
	EmitVertex();
	EndPrimitive();
}