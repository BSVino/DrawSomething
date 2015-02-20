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

#include "matrix.h"

#include "quaternion.h"
#include "common.h"
#include "geometry.h"

mat4::mat4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	Init(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

mat4::mat4(const mat4& i)
{
	m[0][0] = i.m[0][0]; m[0][1] = i.m[0][1]; m[0][2] = i.m[0][2]; m[0][3] = i.m[0][3];
	m[1][0] = i.m[1][0]; m[1][1] = i.m[1][1]; m[1][2] = i.m[1][2]; m[1][3] = i.m[1][3];
	m[2][0] = i.m[2][0]; m[2][1] = i.m[2][1]; m[2][2] = i.m[2][2]; m[2][3] = i.m[2][3];
	m[3][0] = i.m[3][0]; m[3][1] = i.m[3][1]; m[3][2] = i.m[3][2]; m[3][3] = i.m[3][3];
}

mat4::mat4(float* values)
{
	memcpy(&m[0][0], values, sizeof(float) * 16);
}

mat4::mat4(const vec3& forward, const vec3& left, const vec3& up, const vec3& position)
{
	SetForwardVector(forward);
	SetLeftVector(left);
	SetUpVector(up);
	SetTranslation(position);

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
}

mat4::mat4(const quaternion& q)
{
	SetRotation(q);
}

mat4::mat4(const eangle& direction, const vec3& position)
{
	SetAngles(direction);
	SetTranslation(position);

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
}

void mat4::Identity()
{
	memset(this, 0, sizeof(mat4));

	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void mat4::Init(const mat4& i)
{
	memcpy(&m[0][0], &i.m[0][0], sizeof(float) * 16);
}

void mat4::Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

bool mat4::IsIdentity() const
{
	if (!(m[0][0] == 1 && m[1][1] == 1 && m[2][2] == 1 && m[3][3] == 1))
		return false;

	if (!(m[0][1] == 0 && m[0][2] == 0 && m[0][3] == 0))
		return false;

	if (!(m[1][0] == 0 && m[1][2] == 0 && m[1][3] == 0))
		return false;

	if (!(m[2][0] == 0 && m[2][1] == 0 && m[2][3] == 0))
		return false;

	if (!(m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0))
		return false;

	return true;
}

mat4 mat4::Transposed() const
{
	mat4 r;
	r.m[0][0] = m[0][0]; r.m[1][0] = m[0][1]; r.m[2][0] = m[0][2]; r.m[3][0] = m[0][3];
	r.m[0][1] = m[1][0]; r.m[1][1] = m[1][1]; r.m[2][1] = m[1][2]; r.m[3][1] = m[1][3];
	r.m[0][2] = m[2][0]; r.m[1][2] = m[2][1]; r.m[2][2] = m[2][2]; r.m[3][2] = m[2][3];
	r.m[0][3] = m[3][0]; r.m[1][3] = m[3][1]; r.m[2][3] = m[3][2]; r.m[3][3] = m[3][3];
	return r;
}

mat4 mat4::operator*(float f) const
{
	mat4 r;

	r.m[0][0] = m[0][0] * f;
	r.m[0][1] = m[0][1] * f;
	r.m[0][2] = m[0][2] * f;
	r.m[0][3] = m[0][3] * f;

	r.m[1][0] = m[1][0] * f;
	r.m[1][1] = m[1][1] * f;
	r.m[1][2] = m[1][2] * f;
	r.m[1][3] = m[1][3] * f;

	r.m[2][0] = m[2][0] * f;
	r.m[2][1] = m[2][1] * f;
	r.m[2][2] = m[2][2] * f;
	r.m[2][3] = m[2][3] * f;

	r.m[3][0] = m[3][0] * f;
	r.m[3][1] = m[3][1] * f;
	r.m[3][2] = m[3][2] * f;
	r.m[3][3] = m[3][3] * f;

	return r;
}

mat4 mat4::operator+(const mat4& t) const
{
	mat4 r;

	r.m[0][0] = m[0][0] + t.m[0][0];
	r.m[0][1] = m[0][1] + t.m[0][1];
	r.m[0][2] = m[0][2] + t.m[0][2];
	r.m[0][3] = m[0][3] + t.m[0][3];

	r.m[1][0] = m[1][0] + t.m[1][0];
	r.m[1][1] = m[1][1] + t.m[1][1];
	r.m[1][2] = m[1][2] + t.m[1][2];
	r.m[1][3] = m[1][3] + t.m[1][3];

	r.m[2][0] = m[2][0] + t.m[2][0];
	r.m[2][1] = m[2][1] + t.m[2][1];
	r.m[2][2] = m[2][2] + t.m[2][2];
	r.m[2][3] = m[2][3] + t.m[2][3];

	r.m[3][0] = m[3][0] + t.m[3][0];
	r.m[3][1] = m[3][1] + t.m[3][1];
	r.m[3][2] = m[3][2] + t.m[3][2];
	r.m[3][3] = m[3][3] + t.m[3][3];

	return r;
}

mat4 mat4::operator-(const mat4& t) const
{
	mat4 r;

	r.m[0][0] = m[0][0] - t.m[0][0];
	r.m[0][1] = m[0][1] - t.m[0][1];
	r.m[0][2] = m[0][2] - t.m[0][2];
	r.m[0][3] = m[0][3] - t.m[0][3];

	r.m[1][0] = m[1][0] - t.m[1][0];
	r.m[1][1] = m[1][1] - t.m[1][1];
	r.m[1][2] = m[1][2] - t.m[1][2];
	r.m[1][3] = m[1][3] - t.m[1][3];

	r.m[2][0] = m[2][0] - t.m[2][0];
	r.m[2][1] = m[2][1] - t.m[2][1];
	r.m[2][2] = m[2][2] - t.m[2][2];
	r.m[2][3] = m[2][3] - t.m[2][3];

	r.m[3][0] = m[3][0] - t.m[3][0];
	r.m[3][1] = m[3][1] - t.m[3][1];
	r.m[3][2] = m[3][2] - t.m[3][2];
	r.m[3][3] = m[3][3] - t.m[3][3];

	return r;
}

void mat4::SetTranslation(const vec3& vecPos)
{
	m[3][0] = vecPos.x;
	m[3][1] = vecPos.y;
	m[3][2] = vecPos.z;
}

void mat4::SetAngles(const eangle& angles)
{
	float sp = sin(angles.p * M_PI / 180);
	float sy = sin(-angles.y * M_PI / 180);
	float sr = sin(angles.r * M_PI / 180);
	float cp = cos(angles.p * M_PI / 180);
	float cy = cos(-angles.y * M_PI / 180);
	float cr = cos(angles.r * M_PI / 180);

	// Forward vector
	m[0][0] = cy*cp;
	m[0][1] = -sy*cp;
	m[0][2] = sp;

	// Left vector
	m[1][0] = sp*sr*cy + cr*sy;
	m[1][1] = cr*cy - sy*sp*sr;
	m[1][2] = -cp*sr;

	// Up vector
	m[2][0] = sr*sy - sp*cr*cy;
	m[2][1] = sp*cr*sy + sr*cy;
	m[2][2] = cp*cr;
}

void mat4::SetRotation(float angle, const vec3& v)
{
	// Normalize beforehand
	TCheck(fabs(v.LengthSqr() - 1) < 0.000001f);

	// c = cos(angle), s = sin(angle), t = (1-c)
	// [ xxt+c   xyt-zs  xzt+ys ]
	// [ yxt+zs  yyt+c   yzt-xs ]
	// [ zxt-ys  zyt+xs  zzt+c  ]

	float x = v.x;
	float y = v.y;
	float z = v.z;

	float c = cos(angle*M_PI / 180);
	float s = sin(angle*M_PI / 180);
	float t = 1 - c;

	m[0][0] = x*x*t + c;
	m[1][0] = x*y*t - z*s;
	m[2][0] = x*z*t + y*s;

	m[0][1] = y*x*t + z*s;
	m[1][1] = y*y*t + c;
	m[2][1] = y*z*t - x*s;

	m[0][2] = z*x*t - y*s;
	m[1][2] = z*y*t + x*s;
	m[2][2] = z*z*t + c;
}

void mat4::SetRotation(const quaternion& q)
{
	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	float x2 = 2 * x*x;
	float y2 = 2 * y*y;
	float z2 = 2 * z*z;

	float xy2 = 2 * x*y;
	float xz2 = 2 * x*z;
	float yz2 = 2 * y*z;
	float xw2 = 2 * x*w;
	float zw2 = 2 * z*w;
	float yw2 = 2 * y*w;

	m[0][0] = 1 - y2 - z2;
	m[1][0] = xz2 + yw2;
	m[2][0] = xy2 - zw2;

	m[0][1] = xz2 - yw2;
	m[1][1] = 1 - x2 - y2;
	m[2][1] = yz2 + xw2;

	m[0][2] = xy2 + zw2;
	m[1][2] = yz2 - xw2;
	m[2][2] = 1 - x2 - z2;
}

void mat4::SetOrientation(const vec3& v, const vec3& up)
{
	vec3 vecDir = v.Normalized();

	vec3 left;
	if (vecDir != up && vecDir != -up)
		left = up.Cross(vecDir).Normalized();
	else
		left = vec3(0, 1, 0);

	SetForwardVector(vecDir);
	SetLeftVector(left);
	SetUpVector(vecDir.Cross(left).Normalized());
}

void mat4::SetScale(const vec3& scale)
{
	m[0][0] = scale.x;
	m[1][1] = scale.y;
	m[2][2] = scale.z;
}

void mat4::SetReflection(const vec3& plane)
{
	// Normalize beforehand or use ::SetReflection()
	TCheck(fabs(plane.LengthSqr() - 1) < 0.000001f);

	m[0][0] = 1 - 2 * plane.x * plane.x;
	m[1][1] = 1 - 2 * plane.y * plane.y;
	m[2][2] = 1 - 2 * plane.z * plane.z;
	m[1][0] = m[0][1] = -2 * plane.x * plane.y;
	m[2][0] = m[0][2] = -2 * plane.x * plane.z;
	m[1][2] = m[2][1] = -2 * plane.y * plane.z;
}

mat4 mat4::ProjectPerspective(float fov, float aspect_ratio, float near, float far)
{
	float right = near * tan(fov * M_PI / 360);
	float left = -right;

	float bottom = left / aspect_ratio;
	float top = right / aspect_ratio;

	return ProjectFrustum(left, right, bottom, top, near, far);
}

mat4 mat4::ProjectFrustum(float left, float right, float bottom, float top, float near, float far)
{
	mat4 m;

	m.Identity();

	float xd = right - left;
	float yd = top - bottom;
	float zd = far - near;

	m.m[0][0] = (2 * near) / xd;
	m.m[1][1] = (2 * near) / yd;

	m.m[2][0] = (right + left) / xd;
	m.m[2][1] = (top + bottom) / yd;
	m.m[2][2] = -(far + near) / zd;
	m.m[2][3] = -1;

	m.m[3][2] = -(2 * far * near) / zd;

	m.m[3][3] = 0;

	return m;
}

mat4 mat4::ProjectOrthographic(float left, float right, float bottom, float top, float near, float far)
{
	mat4 m;

	m.Identity();

	float xd = right - left;
	float yd = top - bottom;
	float zd = far - near;

	m.m[0][0] = 2.0f / xd;
	m.m[1][1] = 2.0f / yd;
	m.m[2][2] = -2.0f / zd;

	m.m[3][0] = -(right + left) / xd;
	m.m[3][1] = -(top + bottom) / yd;
	m.m[3][2] = -(far + near) / zd;

	return m;
}

mat4 mat4::ConstructCameraView(const vec3& position, const vec3& vecDirection, const vec3& up)
{
	mat4 m;

	m.Identity();

	TCheck(fabs(vecDirection.LengthSqr() - 1) < 0.0001f);

	vec3 cam_side = vecDirection.Cross(up).Normalized();
	vec3 cam_up = cam_side.Cross(vecDirection);

	m.SetForwardVector(vec3(cam_side.x, cam_up.x, -vecDirection.x));
	m.SetLeftVector(vec3(cam_side.y, cam_up.y, -vecDirection.y));
	m.SetUpVector(vec3(cam_side.z, cam_up.z, -vecDirection.z));

	m.AddTranslation(-position);

	return m;
}

vec3 mat4::UnProjectPoint(mat4 projection, mat4 camera, float viewport_w, float viewport_h, vec3 screen_position)
{
	vec4 v(screen_position.x, viewport_h - screen_position.y, screen_position.z, 1.0);

	v.x = (v.x /*- viewport_x*/) / viewport_w;
	v.y = (v.y /*- viewport_y*/) / viewport_h;

	/* Map to range -1 to 1 */
	v.x = v.x * 2 - 1;
	v.y = v.y * 2 - 1;
	v.z = v.z * 2 - 1;

	v = (projection * camera).Inverted() * v;

	if (v.w == 0.0)
		return vec3(0, 0, 0);

	return vec3(v.x, v.y, v.z) / v.w;
}

mat4 mat4::operator+=(const vec3& v)
{
	m[3][0] += v.x;
	m[3][1] += v.y;
	m[3][2] += v.z;

	return *this;
}

mat4 mat4::operator-(const vec3& v) const
{
	mat4 r = *this;

	r.m[3][0] -= v.x;
	r.m[3][1] -= v.y;
	r.m[3][2] -= v.z;

	return r;
}

mat4 mat4::operator+=(const eangle& a)
{
	mat4 r;
	r.SetAngles(a);
	(*this) *= r;

	return *this;
}

mat4 mat4::operator*(const mat4& t) const
{
	mat4 r;

	// [a b c d][A B C D]   [aA+bE+cI+dM
	// [e f g h][E F G H] = [eA+fE+gI+hM ...
	// [i j k l][I J K L]
	// [m n o p][M N O P]

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			r.m[i][j] = m[0][j] * t.m[i][0] + m[1][j] * t.m[i][1] + m[2][j] * t.m[i][2] + m[3][j] * t.m[i][3];
	}

	return r;
}

mat4 mat4::operator*=(const mat4& t)
{
	*this = (*this)*t;

	return *this;
}

bool mat4::operator==(const mat4& t) const
{
	float flEp = 0.000001f;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

bool mat4::Equals(const mat4& t, float flEp) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

mat4 mat4::AddTranslation(const vec3& v)
{
	mat4 r;
	r.SetTranslation(v);
	(*this) *= r;

	return *this;
}

mat4 mat4::AddAngles(const eangle& a)
{
	mat4 r;
	r.SetAngles(a);
	(*this) *= r;

	return *this;
}

mat4 mat4::AddScale(const vec3& scale)
{
	mat4 r;
	r.SetScale(scale);
	(*this) *= r;

	return *this;
}

mat4 mat4::AddReflection(const vec3& v)
{
	mat4 r;
	r.SetReflection(v);
	(*this) *= r;

	return *this;
}

vec3 mat4::GetTranslation() const
{
	return vec3((float*)&m[3][0]);
}

eangle mat4::GetAngles() const
{
#ifdef _DEBUG
	// If any of the below is not true then you have a matrix that has been scaled or reflected or something and it won't work to try to pull its Eulers
	bool b = fabs(GetForwardVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TCheck(b);
		return eangle(0, 0, 0);
	}

	b = fabs(GetUpVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TCheck(b);
		return eangle(0, 0, 0);
	}

	b = fabs(GetLeftVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TCheck(b);
		return eangle(0, 0, 0);
	}

	b = GetForwardVector().Cross(GetLeftVector()).Equals(GetUpVector(), 0.001f);
	if (!b)
	{
		TCheck(b);
		return eangle(0, 0, 0);
	}
#endif

	if (m[0][2] > 0.999999f)
		return eangle(asin(Clamp(m[0][2], -1.0f, 1.0f)) * 180 / M_PI, -atan2(m[1][0], m[1][1]) * 180 / M_PI, 0);
	else if (m[0][2] < -0.999999f)
		return eangle(asin(Clamp(m[0][2], -1.0f, 1.0f)) * 180 / M_PI, -atan2(m[1][0], m[1][1]) * 180 / M_PI, 0);

	// Clamp to [-1, 1] looping
	float flPitch = fmod(m[0][2], 2.0f);
	if (flPitch > 1)
		flPitch -= 2;
	else if (flPitch < -1)
		flPitch += 2;

	return eangle(asin(flPitch) * 180 / M_PI, -atan2(-m[0][1], m[0][0]) * 180 / M_PI, atan2(-m[1][2], m[2][2]) * 180 / M_PI);
}

vec3 mat4::GetScale() const
{
	vec3 r;
	r.x = GetForwardVector().Length();
	r.y = GetLeftVector().Length();
	r.z = GetUpVector().Length();
	return r;
}

vec3 mat4::operator*(const vec3& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+x dX+eY+fZ+y gX+hY+iZ+z]
	// [g h i z][Z]
	//          [1]

	vec3 r;
	r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
	r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
	r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
	return r;
}

const ray mat4::operator*(const ray& r) const
{
	ray result;
	result.m_dir = TransformVector(r.m_dir);
	result.m_pos = (*this) * r.m_pos;
	return result;
}

vec3 mat4::TransformVector(const vec3& v) const
{
	// [a b c][X] 
	// [d e f][Y] = [aX+bY+cZ dX+eY+fZ gX+hY+iZ]
	// [g h i][Z]

	vec3 r;
	r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z;
	r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z;
	r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z;
	return r;
}

vec4 mat4::operator*(const vec4& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+xW dX+eY+fZ+yW gX+hY+iZ+zW jX+kY+lZ+mW]
	// [g h i z][Z]
	// [j k l m][W]

	vec4 r;
	r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
	r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
	r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
	r.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
	return r;
}

vec4 mat4::GetColumn(int i) const
{
	return vec4(m[i][0], m[i][1], m[i][2], m[i][3]);
}

void mat4::SetColumn(int i, const vec4& column)
{
	m[i][0] = column.x;
	m[i][1] = column.y;
	m[i][2] = column.z;
	m[i][3] = column.w;
}

void mat4::SetColumn(int i, const vec3& column)
{
	m[i][0] = column.x;
	m[i][1] = column.y;
	m[i][2] = column.z;
}

void mat4::SetForwardVector(const vec3& v)
{
	m[0][0] = v.x;
	m[0][1] = v.y;
	m[0][2] = v.z;
}

void mat4::SetLeftVector(const vec3& v)
{
	m[1][0] = v.x;
	m[1][1] = v.y;
	m[1][2] = v.z;
}

void mat4::SetUpVector(const vec3& v)
{
	m[2][0] = v.x;
	m[2][1] = v.y;
	m[2][2] = v.z;
}

// Not a true inversion, only works if the matrix is a translation/rotation matrix.
void mat4::InvertRT()
{
	TCheck(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001f);
	TCheck(fabs(GetLeftVector().LengthSqr() - 1) < 0.00001f);
	TCheck(fabs(GetUpVector().LengthSqr() - 1) < 0.00001f);

	mat4 t;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			t.m[h][v] = m[v][h];

	vec3 translation = GetTranslation();

	Init(t);

	SetTranslation(t*(-translation));
}

mat4 mat4::InvertedRT() const
{
	TCheck(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001f);
	TCheck(fabs(GetLeftVector().LengthSqr() - 1) < 0.00001f);
	TCheck(fabs(GetUpVector().LengthSqr() - 1) < 0.00001f);

	mat4 r;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			r.m[h][v] = m[v][h];

	r.SetTranslation(r*(-GetTranslation()));

	return r;
}

const mat4 mat4::Inverted() const
{
	mat4 r;

	float D;

	r.mm[0] =   mm[5]*mm[10]*mm[15] - mm[5]*mm[11]*mm[14] - mm[9]*mm[6]*mm[15]
	          + mm[9]*mm[7]*mm[14]  + mm[13]*mm[6]*mm[11] - mm[13]*mm[7]*mm[10];
	r.mm[4] =  -mm[4]*mm[10]*mm[15] + mm[4]*mm[11]*mm[14] + mm[8]*mm[6]*mm[15]
	          - mm[8]*mm[7]*mm[14]  - mm[12]*mm[6]*mm[11] + mm[12]*mm[7]*mm[10];
	r.mm[8] =   mm[4]*mm[9]*mm[15]  - mm[4]*mm[11]*mm[13] - mm[8]*mm[5]*mm[15]
	          + mm[8]*mm[7]*mm[13]  + mm[12]*mm[5]*mm[11] - mm[12]*mm[7]*mm[9];
	r.mm[12] = -mm[4]*mm[9]*mm[14]  + mm[4]*mm[10]*mm[13] + mm[8]*mm[5]*mm[14]
	          - mm[8]*mm[6]*mm[13]  - mm[12]*mm[5]*mm[10] + mm[12]*mm[6]*mm[9];
	r.mm[1] =  -mm[1]*mm[10]*mm[15] + mm[1]*mm[11]*mm[14] + mm[9]*mm[2]*mm[15]
	          - mm[9]*mm[3]*mm[14]  - mm[13]*mm[2]*mm[11] + mm[13]*mm[3]*mm[10];
	r.mm[5] =   mm[0]*mm[10]*mm[15] - mm[0]*mm[11]*mm[14] - mm[8]*mm[2]*mm[15]
	          + mm[8]*mm[3]*mm[14]  + mm[12]*mm[2]*mm[11] - mm[12]*mm[3]*mm[10];
	r.mm[9] =  -mm[0]*mm[9]*mm[15]  + mm[0]*mm[11]*mm[13] + mm[8]*mm[1]*mm[15]
	          - mm[8]*mm[3]*mm[13]  - mm[12]*mm[1]*mm[11] + mm[12]*mm[3]*mm[9];
	r.mm[13] =  mm[0]*mm[9]*mm[14]  - mm[0]*mm[10]*mm[13] - mm[8]*mm[1]*mm[14]
	          + mm[8]*mm[2]*mm[13]  + mm[12]*mm[1]*mm[10] - mm[12]*mm[2]*mm[9];
	r.mm[2] =   mm[1]*mm[6]*mm[15]  - mm[1]*mm[7]*mm[14]  - mm[5]*mm[2]*mm[15]
	          + mm[5]*mm[3]*mm[14]  + mm[13]*mm[2]*mm[7]  - mm[13]*mm[3]*mm[6];
	r.mm[6] =  -mm[0]*mm[6]*mm[15]  + mm[0]*mm[7]*mm[14]  + mm[4]*mm[2]*mm[15]
	          - mm[4]*mm[3]*mm[14]  - mm[12]*mm[2]*mm[7]  + mm[12]*mm[3]*mm[6];
	r.mm[10] =  mm[0]*mm[5]*mm[15]  - mm[0]*mm[7]*mm[13]  - mm[4]*mm[1]*mm[15]
	          + mm[4]*mm[3]*mm[13]  + mm[12]*mm[1]*mm[7]  - mm[12]*mm[3]*mm[5];
	r.mm[14] = -mm[0]*mm[5]*mm[14]  + mm[0]*mm[6]*mm[13]  + mm[4]*mm[1]*mm[14]
	          - mm[4]*mm[2]*mm[13]  - mm[12]*mm[1]*mm[6]  + mm[12]*mm[2]*mm[5];
	r.mm[3] =  -mm[1]*mm[6]*mm[11]  + mm[1]*mm[7]*mm[10]  + mm[5]*mm[2]*mm[11]
	          - mm[5]*mm[3]*mm[10]  - mm[9]*mm[2]*mm[7]   + mm[9]*mm[3]*mm[6];
	r.mm[7] =   mm[0]*mm[6]*mm[11]  - mm[0]*mm[7]*mm[10]  - mm[4]*mm[2]*mm[11]
	          + mm[4]*mm[3]*mm[10]  + mm[8]*mm[2]*mm[7]   - mm[8]*mm[3]*mm[6];
	r.mm[11] = -mm[0]*mm[5]*mm[11]  + mm[0]*mm[7]*mm[9]   + mm[4]*mm[1]*mm[11]
	          - mm[4]*mm[3]*mm[9]   - mm[8]*mm[1]*mm[7]   + mm[8]*mm[3]*mm[5];
	r.mm[15] =  mm[0]*mm[5]*mm[10]  - mm[0]*mm[6]*mm[9]   - mm[4]*mm[1]*mm[10]
	          + mm[4]*mm[2]*mm[9]   + mm[8]*mm[1]*mm[6]   - mm[8]*mm[2]*mm[5];

	D = mm[0] * r.mm[0] + mm[1] * r.mm[4] + mm[2] * r.mm[8] + mm[3] * r.mm[12];

	if (D == 0)
		return mat4();

	for (int i = 0; i < 16; i++)
		r.mm[i] = r.mm[i] / D;

	return r;
}

float mat4::Trace() const
{
	return m[0][0] + m[1][1] + m[2][2];
}
