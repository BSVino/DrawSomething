in vec3 frag_position;

vec4 fragment_program()
{
	return vec4(AddFog(vec3(1.0, 1.0, 1.0), frag_position, u_camera), 1.0);
}
