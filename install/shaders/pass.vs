in vec3 vert_position;
in vec3 vert_normal;
in vec2 vert_texcoord0;
in vec3 vert_color;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord0;
out vec3 frag_color;

vec4 vertex_program()
{
	frag_position = (u_global * vec4(vert_position, 1.0)).xyz;
	frag_normal = vert_normal;
	frag_texcoord0 = vec2(vert_texcoord0.x, 1.0-vert_texcoord0.y);
	frag_color = vert_color;

	return u_projection * u_view * u_global * vec4(vert_position, 1.0);
}
