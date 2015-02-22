uniform sampler2D u_diffuse_sampler;
uniform vec4 u_color;
uniform vec4 u_diffuse;
uniform bool u_use_normals;

uniform float u_alpha;

in vec3 frag_position;
in vec2 frag_texcoord0;
in vec3 frag_normal;
in vec3 frag_color;

vec4 fragment_program()
{
	vec4 output_color = vec4(frag_color, 1.0);//u_color * u_diffuse;

//	output_color *= texture(u_diffuse_sampler, frag_texcoord0);

//	output_color.a *= u_alpha;

/*	if (u_use_normals)
	{
		vec3 light = normalize(vec3(0.5, 0.5, -1.0));
		output_color.rgb *= RemapValClamped(dot(light, frag_normal), -1.0, 1.0, 0.4, 0.8);
	}*/

//	if (output_color.a < 0.01)
//		discard;

	return vec4(AddFog(output_color.xyz, frag_position, u_camera), 1.0);
}
