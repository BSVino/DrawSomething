in vec3 frag_position;

vec4 fragment_program()
{
	vec3 line_color = vec3(1.0, 1.0, 1.0);

	vec3 position_direction = frag_position - u_camera;
	float position_direction_length = length(position_direction);

	vec3 projected_onto_view = dot(u_camera_direction, position_direction) * u_camera_direction;

	float projection_ratio = 0.5 / length(projected_onto_view);

	float distance_to_plane = position_direction_length * projection_ratio;

	vec3 fadeout_color = mix(line_color, vec3(0.8, 0.8, 0.8), smoothstep(-0.05, 0.05, position_direction_length - distance_to_plane));

	return vec4(AddFog(fadeout_color, frag_position, u_camera), 1.0);
}
