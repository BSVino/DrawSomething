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

#include "quaternion.h"

#include "vector.h"
#include "matrix.h"

quaternion::quaternion()
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}

quaternion::quaternion(float X, float Y, float Z, float W)
{
	x = X;
	y = Y;
	z = Z;
	w = W;
}

quaternion::quaternion(const quaternion& q)
{
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
}

quaternion::quaternion(const mat4& m)
{
	// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
	// article "Quaternion Calculus and Fast Animation".

	float trace = m.Trace();
	float root;

	if (trace > 0)
	{
		// |w| > 1/2, may as well choose w > 1/2
		root = sqrt(trace + 1);  // 2w

		w = root / 2;

		root = 0.5f / root;  // 1/(4w)
		x = (m.m[2][1] - m.m[1][2])*root;
		y = (m.m[1][0] - m.m[0][1])*root;
		z = (m.m[0][2] - m.m[2][0])*root;
	}
	else
	{
		const int NEXT[3] = { 1, 2, 0 };

		// |w| <= 1/2
		int i = 0;
		if (m.m[2][2] > m.m[0][0])
			i = 1;
		if (m.m[1][1] > m.m[i][i])
			i = 2;
		int j = NEXT[i];
		int k = NEXT[j];

		root = sqrt(m.m[i][i] - m.m[j][j] - m.m[k][k] + 1);
		float* quat[3] = { &x, &y, &z };
		*quat[i] = root / 2;
		root = 0.5f / root;
		w = (m.m[j][k] - m.m[k][j])*root;
		*quat[j] = (m.m[i][j] + m.m[j][i])*root;
		*quat[k] = (m.m[i][k] + m.m[k][i])*root;
	}
}

eangle quaternion::GetAngles() const
{
	float sqx = x*x;
	float sqy = y*y;
	float sqz = z*z;
	float sqw = w*w;

	eangle r;
	r.p = atan2(2 * (y*z + x*w), (-sqx - sqy + sqz + sqw)) * 180 / M_PI;
	r.y = asin(-2 * (x*z - y*w)) * 180 / M_PI;
	r.r = atan2(2 * (x*y + z*w), (sqx - sqy - sqz + sqw)) * 180 / M_PI;
	return r;
}

void quaternion::SetAngles(const eangle& a)
{
	float c1 = cos(-a.y / 2 * M_PI / 180);
	float s1 = sin(-a.y / 2 * M_PI / 180);
	float c2 = cos(a.p / 2 * M_PI / 180);
	float s2 = sin(a.p / 2 * M_PI / 180);
	float c3 = cos(a.r / 2 * M_PI / 180);
	float s3 = sin(a.r / 2 * M_PI / 180);

	float c1c2 = c1*c2;
	float s1s2 = s1*s2;

	x = c1c2*s3 + s1s2*c3;
	y = s1*c2*c3 + c1*s2*s3;
	z = c1*s2*c3 - s1*c2*s3;
	w = c1c2*c3 - s1s2*s3;
}

void quaternion::SetRotation(float angle, const vec3& v)
{
	float half_angle = angle / 2;
	float sin_half_angle = sin(half_angle);

	x = sin_half_angle*v.x;
	y = sin_half_angle*v.y;
	z = sin_half_angle*v.z;

	w = cos(half_angle);
}

void quaternion::Normalize()
{
	float m = sqrt(w*w + x*x + y*y + z*z);
	w = w / m;
	x = x / m;
	y = y / m;
	z = z / m;
}

quaternion quaternion::operator*(const quaternion& q)
{
	quaternion r;
	r.w = w*q.w - x*q.x - y*q.y - z*q.z;
	r.x = w*q.x + x*q.w + y*q.z - z*q.y;
	r.y = w*q.y + y*q.w + z*q.x - x*q.z;
	r.z = w*q.z + z*q.w + x*q.y - y*q.x;
	return r;
}

const quaternion& quaternion::operator*=(const quaternion& q)
{
	w = w*q.w - x*q.x - y*q.y - z*q.z;
	x = w*q.x + x*q.w + y*q.z - z*q.y;
	y = w*q.y + y*q.w + z*q.x - x*q.z;
	z = w*q.z + z*q.w + x*q.y - y*q.x;
	return *this;
}
