uniform sampler2D u_source_sampler;
uniform float u_brightness;
uniform float u_scale;

in vec2 frag_texcoord0;

vec4 fragment_program()
{
	vec4 source_color = texture(u_source_sampler, frag_texcoord0);

	float value = source_color.x;
	if (source_color.y > value)
		value = source_color.y;
	if (source_color.z > value)
		value = source_color.z;

	if (value < u_brightness && value > u_brightness - 0.2)
	{
		float strength = RemapVal(value, u_brightness - 0.2, u_brightness, 0.0, 1.0);
		source_color = source_color*strength;
	}
	else if (value < u_brightness - 0.2)
		source_color = vec4(0.0, 0.0, 0.0, 0.0);

	vec4 output_color = source_color*u_scale;
	output_color.a = 1.0;

	return output_color;
}
