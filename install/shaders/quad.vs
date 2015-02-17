in vec3 vert_position;
in vec2 vert_texcoord0;

out vec2 frag_texcoord0;

vec4 vertex_program()
{
	frag_texcoord0 = vec2(vert_texcoord0.x, 1.0-vert_texcoord0.y);

	return vec4(vert_position, 1.0);
}
