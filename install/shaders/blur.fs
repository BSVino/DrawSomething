uniform sampler2D u_source_sampler;
uniform float u_coefficients[3];
uniform float u_offsetx;
uniform float u_offsety;

in vec2 frag_texcoord0;

vec4 fragment_program()
{
	vec2 offset = vec2(u_offsetx, u_offsety);

	vec4 color_sum;
	color_sum  = u_coefficients[0] * texture(u_source_sampler, frag_texcoord0 - offset);
	color_sum += u_coefficients[1] * texture(u_source_sampler, frag_texcoord0);
	color_sum += u_coefficients[2] * texture(u_source_sampler, frag_texcoord0 + offset);

	return color_sum;
}
