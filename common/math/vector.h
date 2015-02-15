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

#pragma once

#include <math.h>

// for size_t
#include <stdlib.h>

#include "common.h"
#include "maths.h"

#ifndef M_PI
#define M_PI 3.14159265359f
#endif

#define M_TAU 6.28318530718f

struct vec2;

struct vec3
{
	float	x, y, z;

	vec3();
	explicit vec3(struct color4 c);
	vec3(float x, float y, float z);
	explicit vec3(const float* xyz);

	// Conversions
	explicit vec3(const vec2& v);

	const vec3 operator-(void) const;

	const vec3 operator+(const vec3& v) const;
	const vec3 operator-(const vec3& v) const;
	const vec3 operator*(float s) const;
	const vec3 operator/(float s) const;

	void operator+=(const vec3 &v);
	void operator-=(const vec3 &v);
	void operator*=(float s);
	void operator/=(float s);

	const vec3 operator*(const vec3& v) const;
	const vec3 operator/(const vec3& v) const;

	friend const vec3 operator*(float s, const vec3& v)
	{
		return vec3(v.x*s, v.y*s, v.z*s);
	}

	friend const vec3 operator/(float s, const vec3& v)
	{
		return vec3(s / v.x, s / v.y, s / v.z);
	}

	bool operator==(const vec3& v) const
	{
		float epsilon = 0.000001f;
		if (fabs(v.x - x) < epsilon && fabs(v.y - y) < epsilon && fabs(v.z - z) < epsilon)
			return true;
		return false;
	}

	bool operator!=(const vec3& v) const
	{
		float epsilon = 0.000001f;
		return fabs(v.x - x) > epsilon || fabs(v.y - y) > epsilon || fabs(v.z - z) > epsilon;
	}

	bool Equals(const vec3& v, float epsilon = 0.000001f) const
	{
		return fabs(AngleDifference(v.x, x)) < epsilon && fabs(AngleDifference(v.y, y)) < epsilon && fabs(AngleDifference(v.z, z)) < epsilon;
	}

	float Length() const;
	float LengthSqr() const;
	float Length2D() const;
	float Length2DSqr() const;
	void Normalize();
	const vec3 Normalized() const;
	const vec3 Flattened() const;
	float Average() const;

	float Distance(const vec3& v) const;
	float DistanceSqr(const vec3& v) const;

	float Dot(const vec3& v) const;
	const vec3 Cross(const vec3& v) const;

	bool IsZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}

	float operator[](int i) const;
	float& operator[](int i);

	float operator[](size_t i) const;
	float& operator[](size_t i);
};

// Euler angles
// Positive pitch looks up, negative looks down.
// Positive yaw rotates like a top to the left, negative to the right.
// Positive roll banks to the right (right wing down, left wing up) negative roll to the left.
struct eangle
{
	float p, y, r;

	eangle();
	eangle(float p, float y, float r);
	explicit eangle(float* pyr);

	const eangle operator+(const eangle& v) const;
	const eangle operator-(const eangle& v) const;

	const eangle operator*(float f) const;
	const eangle operator/(float f) const;

	bool operator==(const eangle& v) const
	{
		float epsilon = 0.000001f;
		return fabs(AngleDifference(v.p, p)) < epsilon && fabs(AngleDifference(v.y, y)) < epsilon && fabs(AngleDifference(v.r, r)) < epsilon;
	}

	bool Equals(const eangle& v, float epsilon = 0.000001f) const
	{
		return fabs(AngleDifference(v.p, p)) < epsilon && fabs(AngleDifference(v.y, y)) < epsilon && fabs(AngleDifference(v.r, r)) < epsilon;
	}

	// Can find equivalence even in dangerous situations such as Gimbal lock.
	bool EqualsExhaustive(const eangle& v, float epsilon = 0.000001f) const;

	friend const eangle operator*(float f, const eangle& a)
	{
		return eangle(a.p*f, a.y*f, a.r*f);
	}

	inline const eangle Lerp(const eangle& from, const eangle& to, float flLerp)
	{
		float p = from.p + (AngleDifference(to.p, from.p) * flLerp);
		float y = from.y + (AngleDifference(to.y, from.y) * flLerp);
		float r = from.r + (AngleDifference(to.r, from.r) * flLerp);
		return eangle(p, y, r);
	}
};

struct vec2
{
	float x, y;

	vec2();
	vec2(float x, float y);
	explicit vec2(vec3 v);

	float Length() const;
	float LengthSqr() const;

	const vec2 operator+(const vec2& v) const;
	const vec2 operator-(const vec2& v) const;
	const vec2 operator*(float s) const;
	const vec2 operator/(float s) const;

	bool operator==(const vec2& v) const
	{
		float epsilon = 0.000001f;
		if (fabs(v.x - x) < epsilon && fabs(v.y - y) < epsilon)
			return true;
		return false;
	}

	bool operator!=(const vec2& v) const
	{
		float epsilon = 0.000001f;
		if (fabs(v.x - x) > epsilon || fabs(v.y - y) > epsilon)
			return true;
		return false;
	}

	void operator+=(const vec2 &v);
	void operator-=(const vec2 &v);

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}
};

struct vec4
{
	float x, y, z, w;

	vec4();
	explicit vec4(const vec3& v);
	vec4(const vec3& v, float w);
	explicit vec4(const struct color4& c);
	vec4(float x, float y, float z, float w);
	explicit vec4(const float* xyzw);

	const vec4 operator+(const vec4& v) const;
	const vec4 operator-(const vec4& v) const;

	const vec4 operator*(float s) const;
	const vec4 operator/(float s) const;

	bool operator==(const vec4& v) const
	{
		float epsilon = 0.000001f;
		return fabs(v.x - x) < epsilon && fabs(v.y - y) < epsilon && fabs(v.z - z) < epsilon && fabs(v.w - w) < epsilon;
	}

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}
};

const vec3 AngleVector(const eangle& a);
const eangle VectorAngles(const vec3& vecForward);
