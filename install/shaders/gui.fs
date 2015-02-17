uniform vec4 u_dimensions;

uniform bool u_use_diffuse;
uniform sampler2D u_diffuse_sampler;
uniform vec4 u_color;

uniform bool u_use_scissor;
uniform vec4 u_scissor;

uniform float u_border;
uniform bool u_highlight;

in vec3 frag_position;
in vec2 frag_texcoord0;

vec4 fragment_program()
{
	vec4 diffuse = u_color;

	if (u_use_diffuse)
		diffuse *= texture(u_diffuse_sampler, frag_texcoord0);

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
			diffuse += vec4(0.5, 0.5, 0.0, 0.0);
		}*/
	}

	if (u_border > 0.0)
	{
		float x = u_dimensions.x;
		float y = u_dimensions.y;
		float w = u_dimensions.z;
		float h = u_dimensions.w;

		float border_strength = 0.2;
		float distance_left = RemapVal(abs(x - frag_position.x), 0.0, u_border, border_strength, 0.0);
		float distance_top = RemapVal(abs(y - frag_position.y), 0.0, u_border, border_strength, 0.0);
		float distance_right = RemapVal(abs(x+w - frag_position.x), 0.0, u_border, border_strength, 0.0);
		float distance_bottom = RemapVal(abs(y+h - frag_position.y), 0.0, u_border, border_strength, 0.0);

		float distance = max(max(max(distance_left, distance_right), distance_top), distance_bottom);
		if (distance > 0.0)
		{
			diffuse.r += distance;
			diffuse.g += distance;
			diffuse.b += distance;
			diffuse.a += distance * 1.0/border_strength;
		}
	}

	vec4 frag_color = diffuse;

	if (u_color.a > 0.0 && u_highlight)
	{
		float y = u_dimensions.y;
		float m = u_dimensions.x + u_dimensions.z/2.0;	// Midpoint

		float distance = abs(y - frag_position.y);
		float glow_cutoff = RemapValSmooth(distance, 250.0, 260.0, 1.0, 0.0);

		float add = RemapValSmooth(distance, 0.0, 250.0, 0.1, 0.01);
		float glow = RemapValSmooth(length_sqr(frag_position.xy - vec2(m, y)), 0.0, 300.0*300.0, 0.08, 0.0);
		if (glow > 0.0)
			add += glow;

		add *= glow_cutoff;
		frag_color.r += add;
		frag_color.g += add;
		frag_color.b += add;

		// Reduce banding.
		float rand = Rand(frag_texcoord0)/200.0;
		frag_color.r += rand;
		frag_color.g += rand;
		frag_color.b += rand;
	}

	return frag_color;
}
