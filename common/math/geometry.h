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
#include <tvector.h>

struct CCollisionResult
{
	bool m_hit;
	bool m_start_inside;
	double m_fraction;
	vec3 m_hit_position;
	vec3 m_hit_normal;

	CCollisionResult()
	{
		m_fraction = 1;
		m_hit = false;
		m_start_inside = false;
	}
};

template <class T>
class TRect
{
public:
	TRect()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	TRect(T X, T Y, T W, T H)
	{
		x = X;
		y = Y;
		w = W;
		h = H;
	}

public:
	T Size() const { return w*h; }

	T Right() const { return x + w; }
	T Bottom() const { return y + h; }

	void SetRight(float r);
	void SetBottom(float b);

	bool Intersects(const TRect<T>& F) const;
	bool Union(const TRect<T>& r);

	void operator/=(T s)
	{
		x /= s;
		y /= s;
		w /= s;
		h /= s;
	}

public:
	T x, y, w, h;
};

template <class T>
void TRect<T>::SetRight(float r)
{
	w = r - x;
}

template <class T>
void TRect<T>::SetBottom(float b)
{
	h = b - y;
}

template <class T>
bool TRect<T>::Intersects(const TRect<T>& r) const
{
	if (x > r.Right())
		return false;

	if (r.x > Right())
		return false;

	if (y > r.Bottom())
		return false;

	if (r.y > Bottom())
		return false;

	return true;
}

template <class T>
bool TRect<T>::Union(const TRect<T>& r)
{
	if (!Intersects(r))
		return false;

	if (r.x > x)
	{
		T right = Right();
		x = r.x;
		SetRight(right);
	}

	if (r.y > y)
	{
		T bottom = Bottom();
		y = r.y;
		SetBottom(bottom);
	}

	if (Right() > r.Right())
		SetRight(r.Right());

	if (Bottom() > r.Bottom())
		SetBottom(r.Bottom());

	return true;
}

typedef TRect<int> Rect;
typedef TRect<float> FRect;

struct ray
{
	vec3 m_pos;
	vec3 m_dir;

	ray() {};
	ray(vec3 position, vec3 direction)
	{
		m_pos = position;
		m_dir = direction;
	}

};

struct AABB
{
	vec3 m_mins;
	vec3 m_maxs;

	AABB() {};
	AABB(const vec3& mins, const vec3& maxs);

	vec3 Center() const;
	vec3 Size() const;

	bool Inside(const AABB& box) const;
	bool Inside(const vec3& point) const;
	bool Inside2D(const vec3& point) const;
	bool Intersects(const AABB& box) const;

	void Expand(const vec3& point);
	void Expand(const AABB& box); // Assumes well formed aabbBox with min < max for x y and z.

	AABB operator+(const AABB& box) const;
	AABB operator*(float s) const;
	bool operator==(const AABB& o) const;
	AABB& operator+=(const vec3& v);
};

bool SameSide(const vec3& p1, const vec3& p2, const vec3& a, const vec3& b);
inline bool PointInTriangle(const vec3& p, const vec3& a, const vec3& b, const vec3& c);
inline float DistanceToLine(const vec3& p, const vec3& v1, const vec3& v2);
inline float DistanceToLineSegment(const vec3& p, const vec3& v1, const vec3& v2, vec3* i = NULL);
inline float DistanceToPlane(const vec3& point, const vec3& plane, const vec3& plane_normal);
inline float DistanceToPlaneSqr(const vec3& point, const vec3& plane, const vec3& plane_normal);
inline float DistanceToQuad(const vec3& p, const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec3& n);
inline float DistanceToPolygon(const vec3& p, tvector<vec3>& v, vec3 n);
inline float DistanceToLineSqr(const vec3& p, const vec3& v1, const vec3& v2);
inline float DistanceToAABBSqr(const vec3& p, const AABB& aabb);
inline float TriangleArea(const vec3& a, const vec3& b, const vec3& c);
inline bool RayIntersectsTriangle(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, vec3* hit = NULL);
inline bool RayIntersectsPlane(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, vec3* hit = NULL);
inline bool RayIntersectsPlane(const ray& r, const vec3& p, const vec3& n, vec3* hit = NULL);
inline bool RayIntersectsQuad(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3, vec3* hit = NULL);
inline bool RayIntersectsAABB(const ray& r, const AABB& b, vec3& intersection);
inline bool RayIntersectsAABB(const ray& r, const AABB& b);
inline bool SegmentIntersectsAABB(const vec3& v1, const vec3& v2, const AABB& b, CCollisionResult& tr);
inline bool LineSegmentIntersectsTriangle(const vec3& s0, const vec3& s1, const vec3& v0, const vec3& v1, const vec3& v2, CCollisionResult& tr);
inline bool LineSegmentIntersectsSphere(const vec3& v1, const vec3& v2, const vec3& s, float flRadius, vec3& point, vec3& normal);
inline bool PointInsideAABB(const AABB& box, const vec3& v);
inline bool	TriangleIntersectsAABB(const AABB& box, const vec3& v0, const vec3& v1, const vec3& v2);
inline bool ConvexHullIntersectsAABB(const AABB& box, const tvector<vec3>& points, const tvector<size_t>& triangles);
inline size_t FindEar(const tvector<vec3>& points);
