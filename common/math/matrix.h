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

#include "vector.h"

// The red pill

struct quaternion;
struct ray;

// This mat4 class is for use in right-handed coordinate spaces with Z up.
// A column is in sequential memory positions (m[0][0], m[0][1], m[0][2], m[0][3])
// A row is in strided positions (m[0][0], m[1][0], m[2][0], m[3][0])
// Its values are stored with Forward/Up/Right vectors residing in the columns 0, 1, 2
// Its translations are stored in the fourth column
// Transformations are done on column vectors on the right

// The notation of each array location
// [00 10 20 30]
// [01 11 21 31]
// [02 12 22 32]
// [03 13 23 33]

// The layout in that notation of the base vectors (f l u t = forward left up translation)
// [fx lx ux tx]
// [fy ly uy ty]
// [fz lz uz tz]
// [fw lw uw tw]

struct mat4
{
	union {
		float m[4][4];
		float mm[16];
	};

	mat4() { Identity(); }
	explicit mat4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
	mat4(const mat4& m);
	explicit mat4(float* values);
	explicit mat4(const vec3& forward, const vec3& left, const vec3& up, const vec3& position = vec3(0, 0, 0));
	explicit mat4(const quaternion& q);
	explicit mat4(const eangle& angDirection, const vec3& position = vec3(0, 0, 0));

	void Identity();
	void Init(const mat4& m);
	void Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

	bool IsIdentity() const;

	mat4 Transposed() const;

	// Simple matrix operations
	mat4 operator*(float f) const;
	mat4 operator+(const mat4& m) const;
	mat4 operator-(const mat4& m) const;

	// Set a transformation
	void SetTranslation(const vec3& pos);
	void SetAngles(const eangle& dir);
	void SetRotation(float angle, const vec3& axis); // Assumes the axis is a normalized vec3.
	void SetRotation(const quaternion& q);
	void SetOrientation(const vec3& dir, const vec3& up = vec3(0, 0, 1));
	void SetScale(const vec3& scale);
	void SetReflection(const vec3& plane_normal); // Reflection around a plane with this normal which passes through the center of the local space.
	// Assumes the plane normal passed in is normalized.

	static mat4 ProjectPerspective(float fov, float aspect_ratio, float near, float far);                     // Just like gluPerspectives
	static mat4 ProjectFrustum(float left, float right, float bottom, float top, float near, float far);      // Just like glFrustum
	static mat4 ProjectOrthographic(float left, float right, float bottom, float top, float near, float far); // Just like glOrtho
	static mat4 ConstructCameraView(const vec3& position, const vec3& direction, const vec3& up);             // Like gluLookAt but a direction parameter instead of target

	// Add a translation
	mat4 operator+=(const vec3& v);
	mat4 operator-(const vec3& v) const;

	// Add a rotation
	mat4 operator+=(const eangle& a);

	// Add a transformation
	mat4 operator*(const mat4& t) const;
	mat4 operator*=(const mat4& t);

	bool operator==(const mat4& t) const;
	bool Equals(const mat4& t, float epsilon = 0.000001f) const;

	// Add a transformation
	mat4 AddTranslation(const vec3& v);
	mat4 AddAngles(const eangle& a);
	mat4 AddScale(const vec3& scale);
	mat4 AddReflection(const vec3& plane_normal);

	vec3   GetTranslation() const;
	eangle GetAngles() const;
	vec3   GetScale() const;

	// Transform a vec3
	vec3 operator*(const vec3& v) const;
	vec3 TransformVector(const vec3& v) const; // Same as homogenous vector with w=0 transform, no translation.
	// You want to use this for directional vectors such as normals and velocities because translations will change their length.
	// It's not immune to scaling though! A matrix with scaling will output a vector of different length than the input.

	vec4 operator*(const vec4& v) const;
	const ray operator*(const ray& r) const;

	// Try not to use these in case the underlying format changes. Use SetXVector() below.
	vec4 GetColumn(int i) const;
	void SetColumn(int i, const vec4& column);
	void SetColumn(int i, const vec3& column);

	void SetForwardVector(const vec3& forward);
	void SetLeftVector(const vec3& left);
	void SetUpVector(const vec3& up);
	vec3 GetForwardVector() const { return vec3((float*)&m[0][0]); }
	vec3 GetLeftVector() const { return vec3((float*)&m[1][0]); }
	vec3 GetUpVector() const { return vec3((float*)&m[2][0]); }

	void InvertRT();
	mat4 InvertedRT() const;

	const mat4 Inverted() const;

	float Trace() const;

	operator float*()
	{
		return(&m[0][0]);
	}

	operator const float*() const
	{
		return(&m[0][0]);
	}
};

