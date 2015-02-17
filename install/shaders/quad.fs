uniform sampler2D u_diffuse_sampler;
uniform vec4 u_color;

in vec2 frag_texcoord0;

vec4 fragment_program()
{
	return u_color * texture(u_diffuse_sampler, frag_texcoord0);
}
