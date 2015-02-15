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

#include "vector.h"

#include "color.h"
#include "matrix.h"

bool eangle::EqualsExhaustive(const eangle& v, float epsilon) const
{
	mat4 m, n;
	m.SetAngles(*this);
	n.SetAngles(v);

	return m.Equals(n, epsilon);
}

void AngleVectors(const eangle& a, vec3* forward, vec3* left, vec3* up)
{
	mat4 m;
	m.SetAngles(a);

	if (forward)
		*forward = m.GetForwardVector();

	if (left)
		*left = m.GetLeftVector();

	if (up)
		*up = m.GetUpVector();
}

vec3::vec3()
{
}

vec3::vec3(color4 c)
{
	x = (float)c.r / 255.0f;
	y = (float)c.g / 255.0f;
	z = (float)c.b / 255.0f;
}

vec3::vec3(float X, float Y, float Z)
	: x(X), y(Y), z(Z)
{
}

vec3::vec3(const float* xyz)
	: x(*xyz), y(*(xyz + 1)), z(*(xyz + 2))
{
}

vec3::vec3(const vec2& v)
	: x((float)v.x), y((float)v.y), z(0)
{
}

const vec3 vec3::operator-() const
{
	return vec3(-x, -y, -z);
}

const vec3 vec3::operator+(const vec3& v) const
{
	return vec3(x + v.x, y + v.y, z + v.z);
}

const vec3 vec3::operator-(const vec3& v) const
{
	return vec3(x - v.x, y - v.y, z - v.z);
}

const vec3 vec3::operator*(float s) const
{
	return vec3(x*s, y*s, z*s);
}

const vec3 vec3::operator/(float s) const
{
	return vec3(x / s, y / s, z / s);
}

void vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

void vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void vec3::operator*=(float s)
{
	x *= s;
	y *= s;
	z *= s;
}

void vec3::operator/=(float s)
{
	x /= s;
	y /= s;
	z /= s;
}

const vec3 vec3::operator*(const vec3& v) const
{
	return vec3(x*v.x, y*v.y, z*v.z);
}

const vec3 vec3::operator/(const vec3& v) const
{
	return vec3(x / v.x, y / v.y, z / v.z);
}

float vec3::Length() const
{
	return sqrt(x*x + y*y + z*z);
}

float vec3::LengthSqr() const
{
	return x*x + y*y + z*z;
}

float vec3::Length2D() const
{
	return sqrt(x*x + y*y);
}

float vec3::Length2DSqr() const
{
	return x*x + y*y;
}

void vec3::Normalize()
{
	float length = Length();
	if (!length)
	{
		TCheck(false);
		*this = vec3(0, 0, 1);
	}
	else
		*this /= length;
}

const vec3 vec3::Normalized() const
{
	float length = Length();
	if (!length)
	{
		TCheck(false);
		return vec3(0, 0, 1);
	}
	else
		return *this / length;
}

const vec3 vec3::Flattened() const
{
	vec3 result(*this);
	result.z = 0;
	return result;
}

float vec3::Average() const
{
	return (x + y + z) / 3;
}

float vec3::Distance(const vec3& v) const
{
	return (*this - v).Length();
}

float vec3::DistanceSqr(const vec3& v) const
{
	return (*this - v).LengthSqr();
}

float vec3::Dot(const vec3& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

const vec3 vec3::Cross(const vec3& v) const
{
	return vec3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

float& vec3::operator[](int i)
{
	return (&x)[i];
}

float vec3::operator[](int i) const
{
	return (&x)[i];
}

float& vec3::operator[](size_t i)
{
	return (&x)[i];
}

float vec3::operator[](size_t i) const
{
	return (&x)[i];
}

eangle::eangle()
{
}

eangle::eangle(float P, float Y, float R)
	: p(P), y(Y), r(R)
{
}

eangle::eangle(float* pyr)
	: p(*pyr), y(*(pyr + 1)), r(*(pyr + 2))
{
}

const eangle eangle::operator+(const eangle& v) const
{
	return eangle(AngleNormalize(p + v.p), AngleNormalize(y + v.y), AngleNormalize(r + v.r));
}

const eangle eangle::operator-(const eangle& v) const
{
	return eangle(AngleDifference(p, v.p), AngleDifference(y, v.y), AngleDifference(r, v.r));
}

const eangle eangle::operator*(float f) const
{
	return eangle(p*f, y*f, r*f);
}

const eangle eangle::operator/(float f) const
{
	return eangle(p / f, y / f, r / f);
}

const vec3 AngleVector(const eangle& a)
{
	vec3 result;

	float p = (float)(a.p * (M_PI * 2 / 360));
	float y = (float)(a.y * (M_PI * 2 / 360));

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);

	result.x = cp*cy;
	result.y = cp*sy;
	result.z = sp;

	return result;
}

const eangle VectorAngles(const vec3& vecForward)
{
	eangle r(0, 0, 0);

	r.p = atan2(vecForward.z, sqrt(vecForward.x*vecForward.x + vecForward.y*vecForward.y)) * (float)(180 / M_PI);
	r.y = atan2(vecForward.y, vecForward.x) * (float)(180 / M_PI);

	return r;
}

vec2::vec2()
{
}

vec2::vec2(float X, float Y)
	: x(X), y(Y)
{
}

vec2::vec2(vec3 v)
	: x(v.x), y(v.y)
{
}

float vec2::Length() const
{
	return sqrt(x*x + y*y);
}

float vec2::LengthSqr() const
{
	return x*x + y*y;
}

const vec2 vec2::operator+(const vec2& v) const
{
	return vec2(x + v.x, y + v.y);
}

const vec2 vec2::operator-(const vec2& v) const
{
	return vec2(x - v.x, y - v.y);
}

const vec2 vec2::operator*(float s) const
{
	return vec2(x*s, y*s);
}

const vec2 vec2::operator/(float s) const
{
	return vec2(x / s, y / s);
}

void vec2::operator+=(const vec2& v)
{
	x += v.x;
	y += v.y;
}

void vec2::operator-=(const vec2& v)
{
	x -= v.x;
	y -= v.y;
}

vec4::vec4()
{
}

vec4::vec4(const vec3& v)
	: x(v.x), y(v.y), z(v.z), w(0)
{
}

vec4::vec4(const vec3& v, float W)
	: x(v.x), y(v.y), z(v.z), w(W)
{
}

vec4::vec4(const color4& c)
	: x(((float)c.r) / 255), y(((float)c.g) / 255), z(((float)c.b) / 255), w(((float)c.a) / 255)
{
}

vec4::vec4(float X, float Y, float Z, float W)
	: x(X), y(Y), z(Z), w(W)
{
}

vec4::vec4(const float* xyzw)
	: x(*xyzw), y(*(xyzw + 1)), z(*(xyzw + 2)), w(*(xyzw + 3))
{
}

const vec4 vec4::operator+(const vec4& v) const
{
	return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

const vec4 vec4::operator-(const vec4& v) const
{
	return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

const vec4 vec4::operator*(float s) const
{
	return vec4(x*s, y*s, z*s, w*s);
}

const vec4 vec4::operator/(float s) const
{
	return vec4(x / s, y / s, z / s, w / s);
}

