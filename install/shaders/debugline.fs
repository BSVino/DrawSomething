in vec3 frag_position;

vec4 fragment_program()
{
	vec3 line_color = vec3(1.0, 1.0, 1.0);

	float distance_to_point = length(frag_position - u_camera);

	vec3 fadeout_color = mix(line_color, vec3(0.8, 0.8, 0.8), smoothstep(-0.1, 0.1, distance_to_point - 0.5));

	return vec4(AddFog(fadeout_color, frag_position, u_camera), 1.0);
}
