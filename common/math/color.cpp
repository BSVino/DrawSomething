/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "color.h"

#include <algorithm>

#include "vector.h"

color4::color4()
{
}

color4::color4(vec3 v)
{
	SetColor((int)(v.x * 255), (int)(v.y * 255), (int)(v.z * 255), 255);
}

color4::color4(int _r, int _g, int _b)
{
	SetColor(_r, _g, _b, 255);
}

color4::color4(int _r, int _g, int _b, int _a)
{
	SetColor(_r, _g, _b, _a);
}

color4::color4(float _r, float _g, float _b)
{
	SetColor(_r, _g, _b, 1.0f);
}

color4::color4(float _r, float _g, float _b, float _a)
{
	SetColor(_r, _g, _b, _a);
}

void color4::SetColor(int _r, int _g, int _b, int _a)
{
	r = (unsigned char)_r;
	g = (unsigned char)_g;
	b = (unsigned char)_b;
	a = (unsigned char)_a;
}

void color4::SetColor(float _r, float _g, float _b, float _a)
{
	r = (unsigned char)(_r * 255);
	g = (unsigned char)(_g * 255);
	b = (unsigned char)(_b * 255);
	a = (unsigned char)(_a * 255);
}

// From: http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
void color4::SetHSL(float hue, float saturation, float lightness)
{
	float hue6 = hue / 60;
	float chroma = (1 - fabs(2 * lightness - 1)) * saturation;
	float x = chroma * (1 - fabs(fmod(hue6, 2.0f) - 1.0f));

	float r1, g1, b1;
	if (hue6 < 1)
	{
		r1 = chroma;
		g1 = x;
		b1 = 0;
	}
	else if (hue6 < 2)
	{
		r1 = x;
		g1 = chroma;
		b1 = 0;
	}
	else if (hue6 < 3)
	{
		r1 = 0;
		g1 = chroma;
		b1 = x;
	}
	else if (hue6 < 4)
	{
		r1 = 0;
		g1 = x;
		b1 = chroma;
	}
	else if (hue6 < 5)
	{
		r1 = x;
		g1 = 0;
		b1 = chroma;
	}
	else
	{
		r1 = chroma;
		g1 = 0;
		b1 = x;
	}

	float m = lightness - chroma / 2;

	*this = color4(r1 + m, g1 + m, b1 + m);
}

// From: http://en.wikipedia.org/wiki/HSL_and_HSV#General_approach
void color4::GetHSL(float& hue, float& saturation, float& lightness)
{
	float R = ((float)r) / 255;
	float G = ((float)g) / 255;
	float B = ((float)b) / 255;

	float M = std::max(std::max(R, G), B);
	float m = std::min(std::min(R, G), B);

	float chroma = M - m;

	if (chroma == 0)
		hue = 0;
	else if (R > G && R > B)
		hue = fmod((G - B) / chroma, 6.0f) * 60;
	else if (G > R && G > B)
		hue = ((B - R) / chroma + 2) * 60;
	else if (B > R && B > G)
		hue = ((R - G) / chroma + 4) * 60;

	lightness = (M + m) / 2;

	saturation = 0;
	if (chroma > 0)
		saturation = chroma / (1 - fabs(2 * lightness - 1));
}

color4 color4::operator-() const
{
	return color4(255 - r, 255 - g, 255 - b, a);
}

color4 color4::operator+(const color4& v) const
{
	return color4(r + v.r, g + v.g, b + v.b, a + v.a);
}

color4 color4::operator-(const color4& v) const
{
	return color4(r - v.r, g - v.g, b - v.b, a - v.a);
}

color4 color4::operator*(float s) const
{
	return color4((int)(r*s), (int)(g*s), (int)(b*s), (int)a);
}

color4 color4::operator/(float s) const
{
	return color4((int)(r / s), (int)(g / s), (int)(b / s), (int)a);
}

void color4::operator+=(const color4& v)
{
	r += v.r;
	g += v.g;
	b += v.b;
}

void color4::operator-=(const color4& v)
{
	r -= v.r;
	g -= v.g;
	b -= v.b;
}

void color4::operator*=(float s)
{
	r = (unsigned char)(s*r);
	g = (unsigned char)(s*g);
	b = (unsigned char)(s*b);
}

void color4::operator/=(float s)
{
	r = (unsigned char)(r / s);
	g = (unsigned char)(g / s);
	b = (unsigned char)(b / s);
}

color4 color4::operator*(const color4& v) const
{
	return color4(r*(float)(v.r / 255), g*(float)(v.g / 255), b*(float)(v.b / 255), a*(float)(v.a / 255));
}
