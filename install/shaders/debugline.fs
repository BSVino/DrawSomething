in vec3 frag_color;

vec4 fragment_program()
{
	return vec4(frag_color, 1.0);
}
