in vec3 vert_position;
in vec2 vert_texcoord0;

out vec3 frag_position;
out vec2 frag_texcoord0;

vec4 vertex_program()
{
	frag_position = vert_position;

	frag_texcoord0 = vert_texcoord0;

	return u_projection * u_view * u_global * vec4(vert_position, 1);
}
