uniform vec4 u_dimensions;

uniform bool u_use_texcoords;
uniform vec4 u_texcoords;

in vec3 vert_position;
in vec2 vert_texcoord0;

out vec3 frag_position;
out vec2 frag_texcoord0;

vec4 vertex_program()
{
	float x = u_dimensions.x;
	float y = u_dimensions.y;
	float w = u_dimensions.z;
	float h = u_dimensions.w;

	frag_position = vec3(x + vert_position.x * w, y + vert_position.y * h, 0);

	if (u_use_texcoords)
	{
		float tx = u_texcoords.x;
		float ty = u_texcoords.y;
		float tw = u_texcoords.z;
		float th = u_texcoords.w;

		frag_texcoord0 = vec2(tx + vert_texcoord0.x * tw, ty + vert_texcoord0.y * th);
	}
	else
		frag_texcoord0 = vert_texcoord0;

	return u_projection * u_global * vec4(frag_position, 1.0);
}
