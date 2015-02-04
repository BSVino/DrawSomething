uniform bool bDiffuse;
uniform sampler2D iDiffuse;
uniform vec4 vecColor;
uniform vec4 vecDiffuse;
uniform bool bNormalsAvailable;

uniform float flAlpha;

in vec3 vecFragmentPosition;
in vec2 vecFragmentTexCoord0;
in vec3 vecFragmentNormal;
in vec3 vecFragmentColor;

uniform mat4 mView;

vec4 fragment_program()
{
	vec4 vecOutputColor = vec4(vecFragmentColor, 1.0);//vecColor * vecDiffuse;

//	if (bDiffuse)
//		vecOutputColor *= texture(iDiffuse, vecFragmentTexCoord0);
//	else
		vecOutputColor *= vec4(0.8, 0.8, 0.8, 1.0);

//	vecOutputColor.a *= flAlpha;

/*	if (bNormalsAvailable)
	{
		vec3 vecLight = normalize(vec3(0.5, 0.5, -1.0));
		vecOutputColor.rgb *= RemapValClamped(dot(vecLight, vecFragmentNormal), -1.0, 1.0, 0.4, 0.8);
	}*/

//	if (vecOutputColor.a < 0.01)
//		discard;

	return vecOutputColor;
}
