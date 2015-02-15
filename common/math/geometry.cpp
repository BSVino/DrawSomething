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

#include "geometry.h"

#include <algorithm>

#include "../common.h"
#include "../tstring.h"

AABB::AABB(const vec3& mins, const vec3& maxs)
{
	m_mins = mins;
	m_maxs = maxs;
}

vec3 AABB::Center() const
{
	return (m_mins + m_maxs) / 2;
}

vec3 AABB::Size() const
{
	return m_maxs - m_mins;
}

bool AABB::Inside(const AABB& box) const
{
	if (m_mins.x < box.m_mins.x)
		return false;

	if (m_mins.y < box.m_mins.y)
		return false;

	if (m_mins.z < box.m_mins.z)
		return false;

	if (m_mins.x > box.m_mins.x)
		return false;

	if (m_mins.y > box.m_mins.y)
		return false;

	if (m_mins.z > box.m_mins.z)
		return false;

	return true;
}

bool AABB::Inside(const vec3& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		float flVI = vecPoint[i];

		if (flVI < m_mins[i] - flEpsilon || flVI > m_maxs[i] + flEpsilon)
			return false;
	}

	return true;
}

bool AABB::Inside2D(const vec3& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	if (vecPoint.x < m_mins.x - flEpsilon || vecPoint.x > m_maxs.x + flEpsilon)
		return false;

	if (vecPoint.y < m_mins.y - flEpsilon || vecPoint.y > m_maxs.y + flEpsilon)
		return false;

	return true;
}

bool AABB::Intersects(const AABB& box) const
{
	if (m_mins.x > box.m_maxs.x)
		return false;

	if (box.m_mins.x > m_maxs.x)
		return false;

	if (m_mins.y > box.m_maxs.y)
		return false;

	if (box.m_mins.y > m_maxs.y)
		return false;

	if (m_mins.z > box.m_maxs.z)
		return false;

	if (box.m_mins.z > m_maxs.z)
		return false;

	return true;
}

void AABB::Expand(const vec3& vecNew)
{
	if (vecNew.x < m_mins.x)
		m_mins.x = vecNew.x;
	else if (vecNew.x > m_maxs.x)
		m_maxs.x = vecNew.x;

	if (vecNew.y < m_mins.y)
		m_mins.y = vecNew.y;
	else if (vecNew.y > m_maxs.y)
		m_maxs.y = vecNew.y;

	if (vecNew.z < m_mins.z)
		m_mins.z = vecNew.z;
	else if (vecNew.z > m_maxs.z)
		m_maxs.z = vecNew.z;
}

void AABB::Expand(const AABB& aabbNew)
{
	if (aabbNew.m_mins.x < m_mins.x)
		m_mins.x = aabbNew.m_mins.x;
	else if (aabbNew.m_maxs.x > m_maxs.x)
		m_maxs.x = aabbNew.m_maxs.x;

	if (aabbNew.m_mins.y < m_mins.y)
		m_mins.y = aabbNew.m_mins.y;
	else if (aabbNew.m_maxs.y > m_maxs.y)
		m_maxs.y = aabbNew.m_maxs.y;

	if (aabbNew.m_mins.z < m_mins.z)
		m_mins.z = aabbNew.m_mins.z;
	else if (aabbNew.m_maxs.z > m_maxs.z)
		m_maxs.z = aabbNew.m_maxs.z;
}

AABB AABB::operator+(const AABB& box) const
{
	AABB r(*this);

	r.m_maxs += box.m_maxs;
	r.m_mins += box.m_mins;

	return r;
}

AABB AABB::operator*(float s) const
{
	AABB r(*this);

	r.m_maxs *= s;
	r.m_mins *= s;

	return r;
}

bool AABB::operator==(const AABB& o) const
{
	return (m_mins == o.m_mins) && (m_maxs == o.m_maxs);
}

AABB& AABB::operator+=(const vec3& v)
{
	m_maxs += v;
	m_mins += v;

	return *this;
}

// Geometry-related functions

bool SameSide(const vec3& p1, const vec3& p2, const vec3& a, const vec3& b)
{
	vec3 ba = b-a;
	vec3 cp1 = ba.Cross(p1-a);
	vec3 cp2 = ba.Cross(p2-a);
	return (cp1.Dot(cp2) > 0);
}

bool PointInTriangle(const vec3& p, const vec3& a, const vec3& b, const vec3& c)
{
	return (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b));
}

float DistanceToLine(const vec3& p, const vec3& v1, const vec3& v2)
{
	vec3 v = v2 - v1;
	vec3 w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	vec3 vb = v1 + v*b;
	return (vb - p).Length();
}

float DistanceToLineSegment(const vec3& p, const vec3& v1, const vec3& v2, vec3* i = NULL)
{
	vec3 v = v2 - v1;
	vec3 w = p - v1;

	float c1 = w.Dot(v);

	if (c1 < 0)
	{
		if (i)
			*i = v1;
		return (v1-p).Length();
	}

	float c2 = v.Dot(v);
	if (c2 < c1)
	{
		if (i)
			*i = v2;
		return (v2-p).Length();
	}

	if (c2 == 0.0f)
		return 0;

	float b = c1/c2;

	vec3 vb = v1 + v*b;

	if (i)
		*i = vb;

	return (vb - p).Length();
}

float DistanceToPlane(const vec3& vecPoint, const vec3& vecPlane, const vec3& vecPlaneNormal)
{
	float sb, sn, sd;

	sn = -vecPlaneNormal.Dot(vecPoint - vecPlane);
	sd = vecPlaneNormal.Dot(vecPlaneNormal);
	sb = sn/sd;

	vec3 b = vecPoint + vecPlaneNormal * sb;
	return (vecPoint - b).Length();
}

float DistanceToPlaneSqr(const vec3& vecPoint, const vec3& vecPlane, const vec3& vecPlaneNormal)
{
	float sb, sn, sd;

	sn = -vecPlaneNormal.Dot(vecPoint - vecPlane);
	sd = vecPlaneNormal.Dot(vecPlaneNormal);
	sb = sn/sd;

	vec3 b = vecPoint + vecPlaneNormal * sb;
	return (vecPoint - b).LengthSqr();
}

#define smaller(l, r) (((l)<(r))?(l):(r))

float DistanceToQuad(const vec3& p, const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec3& n)
{
	float flPlaneDistance = DistanceToPlane(p, v1, n);

	if (PointInTriangle(p, v1, v2, v3))
		return flPlaneDistance;

	if (PointInTriangle(p, v1, v3, v4))
		return flPlaneDistance;

	float flClosestPointSqr = (v1 - p).LengthSqr();

	float flV2Sqr = (v2 - p).LengthSqr();
	float flV3Sqr = (v3 - p).LengthSqr();
	float flV4Sqr = (v4 - p).LengthSqr();

	flClosestPointSqr = smaller(flClosestPointSqr, smaller(flV2Sqr, smaller(flV3Sqr, flV4Sqr)));

	float flClosestPoint = sqrt(flClosestPointSqr);

	float flV12 = DistanceToLineSegment(p, v1, v2);
	float flV23 = DistanceToLineSegment(p, v2, v3);
	float flV34 = DistanceToLineSegment(p, v3, v4);
	float flV41 = DistanceToLineSegment(p, v4, v1);

	flClosestPoint = smaller(flClosestPoint, smaller(flV12, smaller(flV23, smaller(flV34, flV41))));

	return flClosestPoint;
}

float DistanceToPolygon(const vec3& p, tvector<vec3>& v, vec3 n)
{
	float flPlaneDistance = DistanceToPlane(p, v[0], n);

	size_t i;

	bool bFoundPoint = false;

	for (i = 0; i < v.size()-2; i++)
	{
		if (PointInTriangle(p, v[0], v[i+1], v[i+2]))
		{
			bFoundPoint = true;
			break;
		}
	}

	if (bFoundPoint)
		return flPlaneDistance;

	float flClosestPoint = -1;
	for (i = 0; i < v.size(); i++)
	{
		float flPointDistance = (v[i] - p).Length();
		if (flClosestPoint == -1 || (flPointDistance < flClosestPoint))
			flClosestPoint = flPointDistance;

		float flLineDistance;
		if (i == v.size() - 1)
			flLineDistance = DistanceToLineSegment(p, v[i], v[0]);
		else
			flLineDistance = DistanceToLineSegment(p, v[i], v[i+1]);

		if (flClosestPoint == -1 || (flLineDistance < flClosestPoint))
			flClosestPoint = flLineDistance;
	}

	return flClosestPoint;
}

float DistanceToLineSqr(const vec3& p, const vec3& v1, const vec3& v2)
{
	vec3 v = v2 - v1;
	vec3 w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	vec3 vb = v1 + v*b;
	return (vb - p).LengthSqr();
}

float DistanceToAABBSqr(const vec3& p, const AABB& aabb)
{
	if (p.x < aabb.m_mins.x)
	{
		if (p.y < aabb.m_mins.y)
		{
			if (p.z < aabb.m_mins.z)
				return p.DistanceSqr(aabb.m_mins);
			else if (p.z > aabb.m_maxs.z)
				return p.DistanceSqr(vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z));
			else
				return DistanceToLineSqr(p, aabb.m_mins, vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z));
		}
		else if (p.y > aabb.m_maxs.y)
		{
			if (p.z < aabb.m_mins.z)
				return p.DistanceSqr(vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return p.DistanceSqr(vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z));
			else
				return DistanceToLineSqr(p, vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z), vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z));
		}
		else
		{
			if (p.z < aabb.m_mins.z)
				return DistanceToLineSqr(p, aabb.m_mins, vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return DistanceToLineSqr(p, vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z), vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_mins, vec3(-1, 0, 0));
		}
	}
	else if (p.x > aabb.m_maxs.x)
	{
		if (p.y < aabb.m_mins.y)
		{
			if (p.z < aabb.m_mins.z)
				return p.DistanceSqr(vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return p.DistanceSqr(vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z));
			else
				return DistanceToLineSqr(p, vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z), vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z));
		}
		else if (p.y > aabb.m_maxs.y)
		{
			if (p.z < aabb.m_mins.z)
				return p.DistanceSqr(vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return p.DistanceSqr(vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z));
			else
				return DistanceToLineSqr(p, vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z), vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z));
		}
		else
		{
			if (p.z < aabb.m_mins.z)
				return DistanceToLineSqr(p, vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z), vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return DistanceToLineSqr(p, vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z), vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_maxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_maxs, vec3(1, 0, 0));
		}
	}
	else
	{
		if (p.y < aabb.m_mins.y)
		{
			if (p.z < aabb.m_mins.z)
				return DistanceToLineSqr(p, aabb.m_mins, vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return DistanceToLineSqr(p, vec3(aabb.m_mins.x, aabb.m_mins.y, aabb.m_maxs.z), vec3(aabb.m_maxs.x, aabb.m_mins.y, aabb.m_maxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_mins, vec3(0, -1, 0));
		}
		else if (p.y > aabb.m_maxs.y)
		{
			if (p.z < aabb.m_mins.z)
				return DistanceToLineSqr(p, vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_mins.z), vec3(aabb.m_maxs.x, aabb.m_maxs.y, aabb.m_mins.z));
			else if (p.z > aabb.m_maxs.z)
				return DistanceToLineSqr(p, vec3(aabb.m_mins.x, aabb.m_maxs.y, aabb.m_maxs.z), aabb.m_maxs);
			else
				return DistanceToPlaneSqr(p, aabb.m_maxs, vec3(0, 1, 0));
		}
		else
		{
			if (p.z < aabb.m_mins.z)
				return DistanceToPlaneSqr(p, aabb.m_mins, vec3(0, 0, -1));
			else if (p.z > aabb.m_maxs.z)
				return DistanceToPlaneSqr(p, aabb.m_maxs, vec3(0, 0, 1));
			else
				return 0;
		}
	}
}

float TriangleArea(const vec3& a, const vec3& b, const vec3& c)
{
	return (a-b).Cross(a-c).Length()/2;
}

bool RayIntersectsTriangle(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, vec3* hit = NULL)
{
	vec3 u = v1 - v0;
	vec3 v = v2 - v0;
	vec3 n = u.Cross(v);

	vec3 w0 = r.m_pos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(r.m_dir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the triangle

	vec3 vecPoint = r.m_pos + r.m_dir*r;
	if (hit)
		*hit = vecPoint;

	float uu = u.Dot(u);
	float uv = u.Dot(v);
	float vv = v.Dot(v);
	vec3 w = vecPoint - v0;
	float wu = w.Dot(u);
	float wv = w.Dot(v);

	float D = uv * uv - uu * vv;

	float s, t;

	s = (uv * wv - vv * wu) / D;
	if (s < 0 || s > 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t < 0 || (s+t) > 1)	// Intersection point is outside the triangle
		return false;

	return true;
}

bool RayIntersectsPlane(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, vec3* hit = NULL)
{
	vec3 u = v1 - v0;
	vec3 v = v2 - v0;
	vec3 n = u.Cross(v);

	vec3 w0 = r.m_pos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(r.m_dir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the plane

	vec3 vecPoint = r.m_pos + r.m_dir*r;
	if (hit)
		*hit = vecPoint;

	return true;
}

bool RayIntersectsPlane(const ray& r, const vec3& p, const vec3& n, vec3* hit = NULL)
{
	vec3 w0 = r.m_pos - p;

	float a = -n.Dot(w0);
	float b = n.Dot(r.m_dir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the plane

	vec3 vecPoint = r.m_pos + r.m_dir*r;
	if (hit)
		*hit = vecPoint;

	return true;
}

bool RayIntersectsQuad(const ray& r, const vec3& v0, const vec3& v1, const vec3& v2, const vec3& v3, vec3* hit = NULL)
{
	if (RayIntersectsTriangle(r, v0, v1, v2, hit))
		return true;

	else return RayIntersectsTriangle(r, v0, v2, v3, hit);
}

bool ClipRay(float flMin, float flMax, float a, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
		return a <= flMax && a >= flMin;

	float umin = (flMin - a)/d;
	float umax = (flMax - a)/d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

bool RayIntersectsAABB(const ray& r, const AABB& b, vec3& vecIntersection)
{
	float tmin = 0;
	float tmax = b.Size().LengthSqr();	// It's a ray so make tmax effectively infinite.
	if (tmax < 1)
		tmax = 100;
	float flDistTbox = (r.m_pos - b.Center()).LengthSqr();
	if (flDistTbox < 1)
		flDistTbox = 100;
	tmax *= flDistTbox * 100;

	if (!ClipRay(b.m_mins.x, b.m_maxs.x, r.m_pos.x, r.m_dir.x, tmin, tmax))
		return false;

	if (!ClipRay(b.m_mins.y, b.m_maxs.y, r.m_pos.y, r.m_dir.y, tmin, tmax))
		return false;

	if (!ClipRay(b.m_mins.z, b.m_maxs.z, r.m_pos.z, r.m_dir.z, tmin, tmax))
		return false;

	vecIntersection = r.m_pos + r.m_dir * tmin;

	return true;
}

bool RayIntersectsAABB(const ray& r, const AABB& b)
{
	vec3 vecIntersection;
	return RayIntersectsAABB(r, b, vecIntersection);
}

bool ClipSegment(float flMin, float flMax, float a, float b, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return !(b < flMin || a > flMax);
		else
			return !(a < flMin || b > flMax);
	}

	float umin = (flMin - a) / d;
	float umax = (flMax - a) / d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

bool SegmentIntersectsAABB(const vec3& v1, const vec3& v2, const AABB& b, CCollisionResult& tr)
{
	float tmin = 0;
	float tmax = 1;

	vec3 vecDir = v2 - v1;

	if (!ClipSegment(b.m_mins.x, b.m_maxs.x, v1.x, v2.x, vecDir.x, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_mins.y, b.m_maxs.y, v1.y, v2.y, vecDir.y, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_mins.z, b.m_maxs.z, v1.z, v2.z, vecDir.z, tmin, tmax))
		return false;

	float flFraction = (tmin < 0) ? 0 : tmin;
	if (flFraction > tr.flFraction)
		return false;

	tr.m_hit = true;
	tr.bStartInside = tmin < 0;
	tr.flFraction = (float)flFraction;
	//tr.vecHit not set and not used.
	//tr.vecNormal not set and not used.

	return true;
}

bool LineSegmentIntersectsTriangle(const vec3& s0, const vec3& s1, const vec3& v0, const vec3& v1, const vec3& v2, CCollisionResult& tr)
{
	vec3 u = v1 - v0;
	vec3 v = v2 - v0;
	vec3 n = u.Cross(v);

	vec3 w0 = s0 - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(s1 - s0);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Segment is parallel
			return true;	// Segment is inside plane
		else
			return false;	// Segment is somewhere else
	}

	float r = a / b;
	if (r < 0)
		return false;		// Segment goes away from the triangle
	if (r > 1)
		return false;		// Segment goes away from the triangle

	if (tr.flFraction < r)
		return false;

	vec3 vecPoint = s0 + (s1-s0)*r;

	float uu = u.Dot(u);
	float uv = u.Dot(v);
	float vv = v.Dot(v);
	vec3 w = vecPoint - v0;
	float wu = w.Dot(u);
	float wv = w.Dot(v);

	float D = uv * uv - uu * vv;

	float s, t;

	s = (uv * wv - vv * wu) / D;
	if (s <= ep || s >= 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t <= ep || (s+t) >= 1)	// Intersection point is outside the triangle
		return false;

	tr.m_hit = true;
	tr.flFraction = r;
	tr.vecHit = vecPoint;
	tr.vecNormal = (v1-v0).Normalized().Cross((v2-v0).Normalized()).Normalized();

	return true;
}

bool LineSegmentIntersectsSphere(const vec3& v1, const vec3& v2, const vec3& s, float flRadius, vec3& vecPoint, vec3& vecNormal)
{
	vec3 vecLine = v2 - v1;
	vec3 vecSphere = v1 - s;

	if (vecLine.LengthSqr() == 0)
	{
		if (vecSphere.LengthSqr() < flRadius*flRadius)
		{
			vecPoint = v1;
			vecNormal = vecSphere.Normalized();
			return true;
		}
		else
			return false;
	}

	float flA = vecLine.LengthSqr();
	float flB = 2 * vecSphere.Dot(vecLine);
	float flC1 = s.LengthSqr() + v1.LengthSqr();
	float flC2 = (s.Dot(v1)*2);
	float flC = flC1 - flC2 - flRadius*flRadius;

	float flBB4AC = flB*flB - 4*flA*flC;
	if (flBB4AC < 0)
		return false;

	float flSqrt = sqrt(flBB4AC);
	float flPlus = (-flB + flSqrt)/(2*flA);
	float flMinus = (-flB - flSqrt)/(2*flA);

	bool bPlusBelow0 = flPlus < 0;
	bool bMinusBelow0 = flMinus < 0;
	bool bPlusAbove1 = flPlus > 1;
	bool bMinusAbove1 = flMinus > 1;

	// If both are above 1 or below 0, then we're not touching the sphere.
	if (bMinusBelow0 && bPlusBelow0 || bPlusAbove1 && bMinusAbove1)
		return false;

	if (bMinusBelow0 && bPlusAbove1)
	{
		// We are inside the sphere.
		vecPoint = v1;
		vecNormal = (v1 - s).Normalized();
		return true;
	}

	if (bMinusAbove1 && bPlusBelow0)
	{
		// We are inside the sphere. Is this even possible? I dunno. I'm putting an assert here to see.
		// If it's still here later that means no.
		TAssert(false);
		vecPoint = v1;
		vecNormal = (v1 - s).Normalized();
		return true;
	}

	// If flPlus is below 1 and flMinus is below 0 that means we started our trace inside the sphere and we're heading out.
	// Don't intersect with the sphere in this case so that things on the inside can get out without getting stuck.
	if (bMinusBelow0 && !bPlusAbove1)
		return false;

	// So at this point, flMinus is between 0 and 1, and flPlus is above 1.
	// In any other case, we intersect with the sphere and we use the flMinus value as the intersection point.
	float flDistance = vecLine.Length();
	vec3 vecDirection = vecLine / flDistance;

	vecPoint = v1 + vecDirection * (flDistance * flMinus);

	// Oftentimes we are slightly stuck inside the sphere. Pull us out a little bit.
	vec3 vecDifference = vecPoint - s;
	float flDifferenceLength = vecDifference.Length();
	vecNormal = vecDifference / flDifferenceLength;
	if (flDifferenceLength < flRadius)
		vecPoint += vecNormal * ((flRadius - flDifferenceLength) + 0.00001f);
	TAssert((vecPoint - s).LengthSqr() >= flRadius*flRadius);

	return true;
}

bool PointInsideAABB( const AABB& box, const vec3& v )
{
	const float flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		float flVI = v[i];

		if (flVI < box.m_mins[i] - flEpsilon || flVI > box.m_maxs[i] + flEpsilon)
			return false;
	}

	return true;
}

bool TriangleIntersectsAABB( const AABB& box, const vec3& v0, const vec3& v1, const vec3& v2)
{
	// Trivial case rejection: If any of the points are inside the box, return true immediately.
	if (box.Inside(v0))
		return true;
	if (box.Inside(v1))
		return true;
	if (box.Inside(v2))
		return true;

	size_t i;

	// Trivial case rejection: If all three points are on one side of the box then the triangle must be outside of it.
	for (i = 0; i < 3; i++)
	{
		float flBoxMax = box.m_maxs[i];
		float flBoxMin = box.m_mins[i];

		float flV0 = v0[i];
		float flV1 = v1[i];
		float flV2 = v2[i];

		if (flV0 > flBoxMax && flV1 > flBoxMax && flV2 > flBoxMax)
			return false;
		if (flV0 < flBoxMin && flV1 < flBoxMin && flV2 < flBoxMin)
			return false;
	}

	CCollisionResult tr1;
	if (SegmentIntersectsAABB(v0, v1, box, tr1))
		return true;

	CCollisionResult tr2;
	if (SegmentIntersectsAABB(v1, v2, box, tr2))
		return true;

	CCollisionResult tr3;
	if (SegmentIntersectsAABB(v0, v2, box, tr3))
		return true;

	vec3 c0 = box.m_mins;
	vec3 c1 = vec3(box.m_mins.x, box.m_mins.y, box.m_maxs.z);
	vec3 c2 = vec3(box.m_mins.x, box.m_maxs.y, box.m_mins.z);
	vec3 c3 = vec3(box.m_mins.x, box.m_maxs.y, box.m_maxs.z);
	vec3 c4 = vec3(box.m_maxs.x, box.m_mins.y, box.m_mins.z);
	vec3 c5 = vec3(box.m_maxs.x, box.m_mins.y, box.m_maxs.z);
	vec3 c6 = vec3(box.m_maxs.x, box.m_maxs.y, box.m_mins.z);
	vec3 c7 = box.m_maxs;

	// Build a list of line segments in the cube to test against the triangle.
	vec3 aLines[32];

	// Bottom four
	aLines[0] = c0;
	aLines[1] = c1;

	aLines[2] = c1;
	aLines[3] = c2;

	aLines[4] = c2;
	aLines[5] = c3;

	aLines[6] = c3;
	aLines[7] = c0;

	// Sides
	aLines[8] = c0;
	aLines[9] = c4;

	aLines[10] = c1;
	aLines[11] = c5;

	aLines[12] = c2;
	aLines[13] = c6;

	aLines[14] = c3;
	aLines[15] = c7;

	// Top
	aLines[16] = c4;
	aLines[17] = c5;

	aLines[18] = c5;
	aLines[19] = c6;

	aLines[20] = c6;
	aLines[21] = c7;

	aLines[22] = c7;
	aLines[23] = c4;

	// Diagonals
	aLines[24] = c0;
	aLines[25] = c6;

	aLines[26] = c1;
	aLines[27] = c7;

	aLines[28] = c2;
	aLines[29] = c4;

	aLines[30] = c3;
	aLines[31] = c5;

	// If any of the segments intersects with the triangle then we have a winner.
	CCollisionResult tr;
	for (i = 0; i < 32; i+=2)
	{
		if (LineSegmentIntersectsTriangle(aLines[i], aLines[i+1], v0, v1, v2, tr))
			return true;
	}

	return false;
}

bool ConvexHullIntersectsAABB(const AABB& box, const tvector<vec3>& avecPoints, const tvector<size_t>& aiTriangles)
{
	TAssert(aiTriangles.size() % 3 == 0);

	vec3 vecCenter = box.Center();
	vec3 n;

	for (size_t i = 0; i < aiTriangles.size(); i += 3)
	{
		const vec3& v1 = avecPoints[aiTriangles[i]];
		const vec3& v2 = avecPoints[aiTriangles[i+1]];
		const vec3& v3 = avecPoints[aiTriangles[i+2]];

		n = (v2-v1).Cross(v3-v1).Normalized();

		if (n.Dot(vecCenter-v1) < 0)
			continue;

		if (!TriangleIntersectsAABB(box, v1, v2, v3))
			return false;
	}

	return true;
}

size_t FindEar(const tvector<vec3>& avecPoints)
{
	size_t iPoints = avecPoints.size();

	// A triangle is always an ear.
	if (iPoints <= 3)
		return 0;

	size_t i;

	vec3 vecFaceNormal;

	// Calculate the face normal.
	for (i = 0; i < iPoints; i++)
	{
		size_t iNext = (i+1)%iPoints;

		vec3 vecPoint = avecPoints[i];
		vec3 vecNextPoint = avecPoints[iNext];

		vecFaceNormal.x += (vecPoint.y - vecNextPoint.y) * (vecPoint.z + vecNextPoint.z);
		vecFaceNormal.y += (vecPoint.z - vecNextPoint.z) * (vecPoint.x + vecNextPoint.x);
		vecFaceNormal.z += (vecPoint.x - vecNextPoint.x) * (vecPoint.y + vecNextPoint.y);
	}

	vecFaceNormal.Normalize();

	for (i = 0; i < iPoints; i++)
	{
		size_t iLast = i==0?iPoints-1:i-1;
		size_t iNext = i==iPoints-1?0:i+1;

		vec3 vecLast = avecPoints[iLast];
		vec3 vecThis = avecPoints[i];
		vec3 vecNext = avecPoints[iNext];

		// Concave ones can not be ears.
		if ((vecLast-vecThis).Cross(vecLast-vecNext).Dot(vecFaceNormal) < 0)
			continue;

		bool bFoundPoint = false;
		for (size_t j = 0; j < iPoints; j++)
		{
			if (j == i || j == iLast || j == iNext)
				continue;

			if (PointInTriangle(avecPoints[j], vecLast, vecThis, vecNext))
			{
				bFoundPoint = true;
				break;
			}
		}

		if (!bFoundPoint)
			return i;
	}

	return 0;
}
