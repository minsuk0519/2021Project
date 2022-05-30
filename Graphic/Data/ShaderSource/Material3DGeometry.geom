#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VERT_OUT {
	vec3 Fragpos;
	vec3 Normal;
} fragnorm[];

out vec3 Fragpos;
out vec3 Normal;

uniform float time;

void main()
{
	Fragpos = fragnorm[0].Fragpos;
	Normal = fragnorm[0].Normal;
	gl_Position = gl_in[0].gl_Position;// + vec4(Normal, 1.0) * 3.0 * (1 / time - 0.2);
	EmitVertex();
	
	Fragpos = fragnorm[1].Fragpos;
	Normal = fragnorm[1].Normal;
	gl_Position = gl_in[1].gl_Position;// + vec4(Normal, 1.0) * 3.0 * (1 / time - 0.2);
	EmitVertex();
	
	Fragpos = fragnorm[2].Fragpos;
	Normal = fragnorm[2].Normal;
	gl_Position = gl_in[2].gl_Position;// + vec4(Normal, 1.0) * 3.0 * (1 / time - 0.2);
	EmitVertex();
	EndPrimitive();
}