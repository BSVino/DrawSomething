uniform sampler2D u_diffuse_sampler;
uniform vec4 u_color;

uniform bool u_use_scissor;
uniform vec4 u_scissor;

in vec3 frag_position;
in vec2 frag_texcoord0;

vec4 fragment_program()
{
	vec4 diffuse = u_color * (vec4(1.0, 1.0, 1.0, texture(u_diffuse_sampler, frag_texcoord0).a));

	if (u_use_scissor)
	{
		if (frag_position.x < u_scissor.x)
			discard;
		if (frag_position.y < u_scissor.y)
			discard;
		if (frag_position.x > u_scissor.x+u_scissor.z)
			discard;
		if (frag_position.y > u_scissor.y+u_scissor.w)
			discard;

		/*if (frag_position.x > u_scissor.x &&
			frag_position.y > u_scissor.y &&
			frag_position.x < u_scissor.x+u_scissor.z &&
			frag_position.y < u_scissor.y+u_scissor.w)
		{
			diffuse += vec4(0.5, 0.5, 0.0, 0.5);
		}*/
	}

	return diffuse;
}
