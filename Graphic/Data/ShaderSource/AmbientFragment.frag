#version 430 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D posTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;

layout (std140) uniform VPmat
{
	mat4 projection;
	mat4 view;
};

void main()
{
	vec3 samples[64] = vec3[]
	(
		vec3(0.03891, -0.009214, 0.08018), vec3(0.06689, 0.05109, 0.01804), vec3(0.001224, 0, 0.0008837),
		vec3(-0.01583, -0.005251, 0.002853), vec3(0.0004769, 0, 0), vec3(0.003806, 0.005441, 0.01623),
		vec3(0.004097, -0.001237, 0.004424), vec3(0.003327, -0.03314, 0.07328), vec3(-0.09295, -0.009855, 0.03058),
		vec3(-0.03582, -0.07785, 0.0495), vec3(-0.07958, -0.02074, 0.007738), vec3(-0.0452, -0.0458, 0.05928),
		vec3(0.01928, 0.01293, 0.01385), vec3(0.09445, -0.01222, 0.08873), vec3(-0.02341, 0.04795, 0.06212),
		vec3(-0.0225, -0.05654, 0.01826), vec3(0.08415, -0.05166, 0.02826), vec3(-0.146, -0.05446, 0.02609),
		vec3(0.02835, 0.007232, 0.008433), vec3(0.08751, -0.006824, 0.02146), vec3(0.08643, -0.03517, 0.05208),
		vec3(0.03564, 0.05004, 0.0706), vec3(0.08952, 0.01053, 0.03675), vec3(-0.03072, 0.08525, 0.03893),
		vec3(0.1545, 0.04345, 0.06328), vec3(0.1652, -0.04981, 0.1219), vec3(-0.2043, 0.01424, 0.04411),
		vec3(0.1234, 0.1534, 0.09627), vec3(-0.03408, 0.08084, 0.02622), vec3(0.03409, 0.01643, 0.06122),
		vec3(0.02202, 0.02535, 0.00757), vec3(0.0153, -0.1396, 0.01635), vec3(0.0146, 0.01077, 0.01573),
		vec3(-0.01347, -0.06347, 0.07413), vec3(-0.02993, -0.1265, 0.1366), vec3(0.1035, -0.2732, 0.2026),
		vec3(0.05302, 0.115, 0.02572), vec3(-0.05079, 0.05011, 0.03825), vec3(0.0891, 0.07246, 0.1571),
		vec3(0.07675, 0.1057, 0.3353), vec3(0.05562, 0.09848, 0.1261), vec3(-0.2777, 0.2092, 0.2441),
		vec3(0.172, 0.07332, 0.1522), vec3(0.0197, 0.03127, 0.06583), vec3(0.1573, 0.3122, 0.02963),
		vec3(-0.2399, -0.4028, 0.06705), vec3(0.1492, 0.1038, 0.03289), vec3(0.1948, 0.02745, 0.1364),
		vec3(-0.05762, -0.00986, 0.07175), vec3(-0.2736, 0.368, 0.2427), vec3(-0.01738, 0.03859, 0.005529),
		vec3(0.4377, 0.03776, 0.4682), vec3(0.09718, 0.3092, 0.1088), vec3(-0.1065, -0.002809, 0.3494),
		vec3(-0.1525, 0.2439, 0.3449), vec3(-0.007045, 0.205, 0.2059), vec3(0.6316, 0.1169, 0.3952),
		vec3(0.03541, -0.08752, 0.05064), vec3(0.04387, 0.02472, 0.02296), vec3(-0.233, 0.08734, 0.12),
		vec3(-0.2106, 0.256, 0.638), vec3(0.05283, 0.2212, 0.7945), vec3(-0.1987, 0.07281, 0.1125),
		vec3(-0.2103, 0.2137, 0.4404)
	);
	

    vec3 FragPos = texture(posTex, TexCoord).xyz;
	
	FragPos = (view * vec4(FragPos, 1.0)).xyz;
	
    vec3 Normal = normalize(inverse(transpose(mat3(view))) * (texture(normalTex, TexCoord).xyz));
	
	vec2 noiseScale = textureSize(posTex, 0) / 4.0;
    vec3 randomVec = normalize(texture(noiseTex, TexCoord * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - Normal * dot(randomVec, Normal));
    vec3 bitangent = normalize(cross(Normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, Normal);
	
    float occlusion = 0.0;
    for(int i = 0; i < 64; ++i)
    {
        vec3 samplePos = TBN * samples[i];
        samplePos = FragPos + samplePos * 0.5; 
		
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
		
		vec3 AA = texture(posTex, offset.xy).xyz;
		AA = (view * vec4(AA, 1.0)).xyz;
        
        float sampleDepth = AA.z;
        
        float rangeCheck = smoothstep(0.0, 1.0, 0.5 / abs(FragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / 64.0);
    
    FragColor = vec4(occlusion, occlusion, occlusion, 1);
    //FragColor = occlusion;
}