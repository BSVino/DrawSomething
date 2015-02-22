in vec3 frag_position;
in vec2 frag_texcoord0;

vec4 fragment_program()
{
	// Just saturate it with fog.
	return vec4(g_fog, 1.0);
}
