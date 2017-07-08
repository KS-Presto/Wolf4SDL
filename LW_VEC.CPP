/*************************************************************************
** Hitler's Mechanical Castle
** Copyright (C) 2017 by LinuxWolf - Team RayCAST
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**************************************************************************
** LinuxWolf Library for Hitler's Mechanical Castle
*************************************************************************/

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include "lw_vec.h"
using namespace lwlib;

BoxData_t lwlib::g_boxData =
{
	{
		{
			{ 1, 0, 0 },
			{ 1, 1, 0 },
			{ 1, 1, 1 },
			{ 1, 0, 1 },
		},
		{
			{ 0, 1, 0 },
			{ 0, 0, 0 },
			{ 0, 0, 1 },
			{ 0, 1, 1 },
		},
		{
			{ 1, 1, 0 },
			{ 0, 1, 0 },
			{ 0, 1, 1 },
			{ 1, 1, 1 },
		},
		{
			{ 0, 0, 0 },
			{ 1, 0, 0 },
			{ 1, 0, 1 },
			{ 0, 0, 1 },
		},
		{
			{ 0, 0, 1 },
			{ 1, 0, 1 },
			{ 1, 1, 1 },
			{ 0, 1, 1 },
		},
		{
			{ 0, 1, 0 },
			{ 1, 1, 0 },
			{ 1, 0, 0 },
			{ 0, 0, 0 },
		},
	}, // faceVertTable
};

unsigned long lwlib::vec3f_quantize(Point3f a)
{
	int i;
	Point3i b;

	for (i = 0; i < 3; i++)
	{
		b.v[i] = lwlib_MIN((int)(a.v[i] * 255.0 + 0.5), 255);
	}

	#ifdef RGB_LSB_FIRST
	return (b.v[0] << 16UL) | (b.v[1] << 8UL) | (b.v[2] << 0UL);
	#else
	return (b.v[0] << 0UL) | (b.v[1] << 8UL) | (b.v[2] << 16UL);
	#endif
}

Vector3f lwlib::vec3f_normal(Point3f a, Point3f b, Point3f c)
{
	double n;
	Vector3f v;

	v = vec3_cross(vec_sub(b, a), vec_sub(c, a));
	lwlib_NormalizeVector(n, v);

	return v;
}

Plane3f lwlib::vec3f_plane(Point3f a, Point3f b, Point3f c)
{
	return plane3f(a, vec3f_normal(a, b, c));
}

int lwlib::vec3f_sphere_side(Sphere3f a, Point3f b, double eps)
{
	double dot, radiusSq;
	b = vec_sub(b, a.origin);
	dot = vec_dot(b, b);
	radiusSq = a.radius * a.radius;
	return dot > radiusSq + eps ? 1 : (dot < radiusSq - eps ? -1 : 0);
}

double lwlib::vec3f_face_area(Face3f_t face, Vector3f normal)
{
	int i;
	Point3f v;
	int N = face.numPoints;

	// 2A = n dot sum(0,N-1) (V(i) x V(i+1))

	v = vec3f_zero();
	for (i = 0; i < N; i++)
	{
		v = vec_add(v, 
			vec3_cross(
				LWLIB_FACE_POINT(face, i), 
				LWLIB_FACE_POINT(face, (i + 1) % N)
				)
			);
	}
	
	return vec_dot(normal, v) / 2.0;
}

Face3f_t lwlib::vec3f_face(void *priv, FacePoint3f_t points, int numPoints)
{
	Face3f_t face;
	face.priv = priv;
	face.points = points;
	face.numPoints = numPoints;
	return face;
}

int lwlib::vec3f_face_side(Face3f_t face, Plane3f plane, double eps)
{
	int i;
	int ret;
	int counted[2] = { 0, 0 };

	for (i = 0; i < face.numPoints; i++)
	{
		ret = vec_side(plane, LWLIB_FACE_POINT(face, i), eps);
		if (ret > 0)
			counted[0]++;
		else if (ret < 0)
			counted[1]++;
	}

	if (counted[0] == 0 && counted[1] == 0)
		return FaceSide::Coplanar;
	if (counted[0] != 0 && counted[1] != 0)
		return FaceSide::Split;
	if (counted[0] != 0 && counted[1] == 0)
		return FaceSide::Front;
	if (counted[0] == 0 && counted[1] != 0)
		return FaceSide::Back;

	assert(0);
}

Box3f lwlib::vec3f_face_box(Face3f_t face)
{
	int i;
	Box3f box = box3f(vec3f_zero(), vec3f_zero());

	for (i = 0; i < face.numPoints; i++)
	{
		if (i == 0)
			box = box3f(LWLIB_FACE_POINT(face, i), LWLIB_FACE_POINT(face, i));
		else
			box = box_expand(box, LWLIB_FACE_POINT(face, i));
	}
	
	return box;
}

Point3f lwlib::vec3f_face_sum(Face3f_t face)
{
	int i;
	Point3f v;
	int N = face.numPoints;

	v = vec3f_zero();
	for (i = 0; i < N; i++)
	{
		v = vec_add(v, LWLIB_FACE_POINT(face, i));
	}
	return v;
}

Point3f lwlib::vec3f_face_center(Face3f_t face)
{
	int N = face.numPoints;

	if (N == 0)
	{
		return vec3f_zero();
	}

	return vec_scale(vec3f_face_sum(face), 1.0 / (double)N);
}

Point3f lwlib::vec3f_face_point(void *priv, int n)
{
	return ((Point3f *)priv)[n];
}

Point3f lwlib::vec3f_quad_lerp(Point3f quad[4], double u, double v)
{
	Point3f lerp;
	lwlib_X(lerp) = lwlib_X(quad[0]) * (1-u)*(1-v) + 
		lwlib_X(quad[1]) * (1-v)*u + 
		lwlib_X(quad[2]) * u*v + 
		lwlib_X(quad[3]) * v*(1-u);
	lwlib_Y(lerp) = lwlib_Y(quad[0]) * (1-u)*(1-v) + 
		lwlib_Y(quad[1]) * (1-v)*u + 
		lwlib_Y(quad[2]) * u*v + 
		lwlib_Y(quad[3]) * v*(1-u);
	lwlib_Z(lerp) = lwlib_Z(quad[0]) * (1-u)*(1-v) + 
		lwlib_Z(quad[1]) * (1-v)*u + 
		lwlib_Z(quad[2]) * u*v +
		lwlib_Z(quad[3]) * v*(1-u);
	return lerp;
}

Point3f lwlib::vec3f_rot90(Point3f a)
{
	return vec3f(-lwlib_Y(a), lwlib_X(a), lwlib_Z(a));
}

Point3f lwlib::vec3f_rot90_anchor(Point3f a, Point3f b)
{
	return vec_add(vec3f_rot90(vec_sub(a, b)), b);
}

Plane3f lwlib::box3f_face_plane(const Box3f box, int facing)
{
	return plane3f(box3_face_vert(box, facing, 0), vec_facing_normal<double, 3>(facing));
}

Box3f lwlib::box3f_rotate(Box3f a, Mat3f b)
{
	int i;
	Box3f c;
	Point3f v;

	for (i = 0; i < 8; i++)
	{
		v = mat_xform(b, box_corner(a, i));
		if (i == 0)
		{
			c = box3f(v, v);
		}
		else
		{
			c = box_expand(c, v);
		}
	}

	return c;
}

int lwlib::ray3f_hit_box(Ray3f ray, Box3f box)
{
	int i, j;
	int m[3];
	int mlen = 0;
	const Point3f a = ray.p[0];
	const Vector3f v = ray.p[1];
	const Point3f bl = box.p[0];
	const Point3f bu = box.p[1];
	Point3f hit;
	double s, t;
	int tryHit = 0;

	for (i = 0; i < 3; i++)
	{
		if 
			(
				(lwlib_C(a, i) <= lwlib_C(bl, i) && lwlib_C(v, i) <= 0.0) ||
				(lwlib_C(a, i) >= lwlib_C(bu, i) && lwlib_C(v, i) >= 0.0)
			)
		{
			return 0;
		}
	}

	if (lwlib_X(a) > lwlib_X(bl) && lwlib_X(a) < lwlib_X(bu) &&
		lwlib_Y(a) > lwlib_Y(bl) && lwlib_Y(a) < lwlib_Y(bu) &&
		lwlib_Z(a) > lwlib_Z(bl) && lwlib_Z(a) < lwlib_Z(bu))
	{
		return 1;
	}

	/* define mapping to the indices of non-zero components in the direction
	 * vector of ray */
	for (i = 0; i < 3; i++)
	{
		s = lwlib_Sign(lwlib_C(v, i));
		if (s != 0.0)
		{
			m[mlen++] = i;
		}
	}

	for (i = 0; i < mlen; i++)
	{
		tryHit = 0;
		if (lwlib_Cm(a, i, m) <= lwlib_Cm(bl, i, m) && lwlib_Cm(v, i, m) > 0)
		{
			t = (lwlib_Cm(bl, i, m) - lwlib_Cm(a, i, m)) / lwlib_Cm(v, i, m);
			tryHit = 1;
		}
		else if (lwlib_Cm(a, i, m) >= lwlib_Cm(bu, i, m) && lwlib_Cm(v, i, m) < 0)
		{
			t = (lwlib_Cm(bu, i, m) - lwlib_Cm(a, i, m)) / lwlib_Cm(v, i, m);
			tryHit = 1;
		}

		if (tryHit)
		{
			hit = vec_add(a, vec_scale(v, t));
			for (j = 0; j < 3; j++)
			{
				if (m[i] == j)
					continue;

				if (!(lwlib_C(hit, j) >= lwlib_C(bl, j) && lwlib_C(hit, j) <= lwlib_C(bu, j)))
					break;
			}

			if (j == 3)
			{
				return 1;
			}
		}
	}

	return 0;
}

int lwlib::ray3f_hit_plane(Ray3f ray, Plane3f plane, Point3f *hit)
{
	double dist;
	double dirProj;
	double t;

	dist = 
		vec_dot(
			vec_sub(ray.p[0], plane.p[0]), 
			plane.p[1]
			);
	dirProj = vec_dot(ray.p[1], plane.p[1]);

	if ((dist > 0 && dirProj < 0) || (dist < 0 && dirProj > 0))
	{
		if (hit)
		{
			t = -dist / dirProj;
			*hit = vec_add(ray.p[0], vec_scale(ray.p[1], t));
		}

		return 1;
	}

	return 0;
}

Sphere3f lwlib::sphere3f(Point3f origin, double radius)
{
	Sphere3f sphere;
	sphere.origin = origin;
	sphere.radius = radius;
	return sphere;
}

int lwlib::hit_line_cone(Line_t line, Cone_t cone, Point3f *hit)
{
	int i;
	double t, delta, gamma, c0, c1, c2, ring;
	Point3f Xt;
	Mat3f D, P, V, M, grad, A;
	double tList[3];
	int tListLen = 0, lowerHit, upperHit;
	double rayProj;

	// zero cone cannot be hit by line
	if (cone.angle == 0.0)
		return 0;
	
	// trivial intersection test using ray angle
	if (cone.trivialTest)
	{
		rayProj = vec_dot(line.direction, cone.axis);
		return rayProj >= cone.minRayProj && rayProj <= cone.maxRayProj;
	}

	assert(cone.angle > 0 && cone.angle < 90);
	gamma = cos(cone.angle * M_PI / 180.0);

	P = mat_col_mat(3, line.point);
	D = mat_col_mat(3, line.direction);

	V = mat_col_mat(3, cone.vertex);
	A = mat_col_mat(3, cone.axis);

	M = 
		mat_sub(
			mat_mult(A, mat_xpose(A)),
			mat_scale(mat3f_ident(3, 3), gamma * gamma)
			);

	grad = mat_sub(P, V);

	c2 = lwlib_X(mat_mult(mat_mult(mat_xpose(D), M), D));
	c1 = lwlib_X(mat_mult(mat_mult(mat_xpose(D), M), grad));
	c0 = lwlib_X(mat_mult(mat_mult(mat_xpose(grad), M), grad));

	if (c2 != 0.0)
	{
		delta = c1 * c1 - c0 * c2;
		// no real roots
		if (delta < 0.0)
			return 0;
		// double root
		else if (delta == 0.0)
		{
			t = -c1 / c2;
			tList[tListLen++] = t;
		}
		// two distinct real roots
		else // delta > 0.0
		{
			t = (-c1 - sqrt(delta)) / c2;
			tList[tListLen++] = t;

			t = (-c1 + sqrt(delta)) / c2;
			tList[tListLen++] = t;
		}
	}
	else // c2 == 0.0
	{
		if (c1 != 0.0)
		{
			t = -c0 / (2 * c1);
			tList[tListLen++] = t;
		}
		// else line is completely contained in cone
	}

	for (i = 0; i < tListLen; i++)
	{
		t = tList[i];
		if (t < 0.0)
			continue;

		Xt = vec_add(line.point, vec_scale(line.direction, t));

		ring = vec_dot(vec_sub(Xt, cone.vertex), cone.axis);
		// handle double cone
		if (ring < 0.0)
			continue;

		lowerHit = (cone.lowerRim == 0.0 || ring >= cone.lowerRim);
		upperHit = (cone.upperRim == 0.0 || ring <= cone.upperRim);
		if (!(lowerHit && upperHit))
			continue;

		// found line-cone intersection
		if (hit != NULL)
			*hit = Xt;
		return 1;
	}

	return 0;
}

bool lwlib::vec3f_box_halfspace_overlap(Box3f box, Plane3f plane, 
	OverlapTest::e test)
{
	int bits;
	Point3f v;

	bits = vec_sign_bits(plane.p[1], 0.0);
	if (test == OverlapTest::Touch)
	{
		bits ^= 0x07;
	}

	v = box_corner(box, bits);
	return vec_side(plane, v, 0.0) <= 0;
}

bool lwlib::vec3f_box_touches_sphere(Box3f box, Sphere3f sphere)
{
	double dmin;
	double r2;
	int i;

	for (i = 0; i < 3; i++)
	{
		if (lwlib_C(sphere.origin, i) + sphere.radius < 
			lwlib_C(lwlib_box_lower(box), i) ||
			lwlib_C(sphere.origin, i) - sphere.radius > 
			lwlib_C(lwlib_box_upper(box), i))
		{
			return false;
		}
	}

	dmin = 0;
	r2 = lwlib_Sq(sphere.radius);
	for (i = 0; i < 3; i++)
	{
		if (lwlib_C(sphere.origin, i) < lwlib_C(lwlib_box_lower(box), i))
		{
			dmin += lwlib_Sq(lwlib_C(sphere.origin, i) - 
				lwlib_C(lwlib_box_lower(box), i));
		}
		else if (lwlib_C(sphere.origin, i) > lwlib_C(lwlib_box_upper(box), i))
		{
			dmin += lwlib_Sq(lwlib_C(sphere.origin, i) - 
				lwlib_C(lwlib_box_upper(box), i));
		}
	}

	if (dmin <= r2)
	{
		return true;
	}

	return false;
}

bool lwlib::vec3f_box_inside_sphere(Box3f box, Sphere3f sphere)
{
	int i;
	Point3f v;
	Point3f boxCenter;
	Vector3f boxHalfSpan;

	boxCenter = box_center(box);
	boxHalfSpan = vec_scale(box_span(box), 0.5);

	for (i = 0; i < 3; i++)
	{
		if (lwlib_C(boxCenter, i) >= lwlib_C(sphere.origin, i))
		{
			lwlib_C(v, i) = lwlib_C(boxCenter, i) + lwlib_C(boxHalfSpan, i);
		}
		else
		{
			lwlib_C(v, i) = lwlib_C(boxCenter, i) - lwlib_C(boxHalfSpan, i);
		}
	}

	v = vec_sub(v, sphere.origin);
	return vec_dot(v, v) <= lwlib_Sq(sphere.radius);
}

bool lwlib::vec3f_box_sphere_overlap(Box3f box, Sphere3f sphere, OverlapTest::e test)
{
	return test == OverlapTest::Touch ? 
		vec3f_box_touches_sphere(box, sphere) :
		vec3f_box_inside_sphere(box, sphere);
}

Point3f lwlib::rgb_to_cielab(Point3f rgb)
{
	int c;

	// rgb to xyz
	for (c = 0; c < 3; c++)
	{
		if (lwlib_C(rgb, c) > 0.04045)
		{
			lwlib_C(rgb, c) = powf((lwlib_C(rgb, c) + 0.055) / 1.055,
				2.4);
		}
		else
		{
			lwlib_C(rgb, c) = lwlib_C(rgb, c) / 12.92;
		}
		lwlib_C(rgb, c) *= 100.0;
	}

	// xyz to cielab
	return vec3f(
		lwlib_X(rgb) * 0.4124 + lwlib_Y(rgb) * 0.3576 + 
			lwlib_Z(rgb) * 0.1805,
		lwlib_X(rgb) * 0.2126 + lwlib_Y(rgb) * 0.7152 + 
			lwlib_Z(rgb) * 0.0722,
		lwlib_X(rgb) * 0.0193 + lwlib_Y(rgb) * 0.1192 + 
			lwlib_Z(rgb) * 0.9505
		);
}
