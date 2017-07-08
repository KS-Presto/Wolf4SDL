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

#ifndef LWLIB_VEC_H
#define LWLIB_VEC_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include "lw_misc.h"

/* common */

#define lwlib_EPSILON 0.00001
#define lwlib_MIN(a,b) ((a) < (b) ? (a) : (b))
#define lwlib_MAX(a,b) ((a) > (b) ? (a) : (b))
#define lwlib_MINMAX(a,b,c) ((c) ? ((a) > (b) ? (a) : (b)) : ((a) < (b) ? (a) : (b)))
#define lwlib_CLIP(x,a,b) lwlib_MIN(lwlib_MAX(x,a), b)

#define lwlib_plane_origin(plane) (plane).p[0]
#define lwlib_plane_normal(plane) (plane).p[1]
#define lwlib_ray_start(ray) (ray).p[0]
#define lwlib_ray_dir(ray) (ray).p[1]

#define lwlib_box_lower(box) (box).p[0]
#define lwlib_box_upper(box) (box).p[1]
#define lwlib_box_extents(box) lwlib_box_lower(box), lwlib_box_upper(box)

#define LWLIB_FACE_POINT(face, n) (face).points((face).priv, (n))

/* vector math */

/* access named component of vector */
#define lwlib_X(a) (a).v[0]
#define lwlib_Y(a) (a).v[1]
#define lwlib_Z(a) (a).v[2]
#define lwlib_W(a) (a).v[3]

/* access numbered component of vector */
#define lwlib_C(a, i) (a).v[i]

/* access mapped, numbered component of vector */ 
#define lwlib_Cm(a, i, m) lwlib_C(a, (m)[i])

#define lwlib_AddVector2(c,a,b) lwlib_X(c)=lwlib_X(a)+lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)+lwlib_Y(b)
#define lwlib_SubVector2(c,a,b) lwlib_X(c)=lwlib_X(a)-lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)-lwlib_Y(b)
#define lwlib_DotVector2(a,b) (lwlib_X(a)*lwlib_X(b) + lwlib_Y(a)*lwlib_Y(b))
#define lwlib_ScaleVector2(c,a,b) lwlib_X(c)=lwlib_X(a)*(b), lwlib_Y(c)=lwlib_Y(a)*(b)
#define lwlib_DivideVector2(c,a,b) lwlib_X(c)=lwlib_X(a)/(b), lwlib_Y(c)=lwlib_Y(a)/(b)
#define lwlib_NormalizeVector2(n,a) \
	( \
		(n)=sqrt(lwlib_DotVector2(a,a)), \
		(n) ? \
			(lwlib_X(a)/=n, lwlib_Y(a)/=n) : \
			0 \
	)
#define lwlib_InitVector2(v,a,b) lwlib_X(v) = (a), lwlib_Y(v) = (b)
#define lwlib_InvertVector2(a,b) lwlib_X(a) = -lwlib_X(b), lwlib_Y(a) = -lwlib_Y(b)
#define lwlib_MultVector2(c,a,b) lwlib_X(c) = lwlib_X(a) * lwlib_X(b), lwlib_Y(c) = lwlib_Y(a) * lwlib_Y(b)
#define lwlib_VectorLength2(v) sqrt(lwlib_DotVector2(v,v))
#define lwlib_VectorLengthSq2(v) lwlib_DotVector2(v,v)
#define lwlib_TransformVector2(a,b,f) lwlib_X(a) = f(lwlib_X(b)), lwlib_Y(a) = f(lwlib_Y(b))
#define lwlib_PackVector2(a) (a)[0], (a)[1]
#define lwlib_UnpackVector2(a, b) (b)[0] = lwlib_X(a), (b)[1] = lwlib_Y(a)
#define lwlib_ExpandVector2(a) lwlib_X(a), lwlib_Y(a)

#define lwlib_vec2_init(x,y) { { (x), (y) } }

#define lwlib_AddVector(c,a,b) lwlib_X(c)=lwlib_X(a)+lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)+lwlib_Y(b), lwlib_Z(c)=lwlib_Z(a)+lwlib_Z(b)
#define lwlib_SubVector(c,a,b) lwlib_X(c)=lwlib_X(a)-lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)-lwlib_Y(b), lwlib_Z(c)=lwlib_Z(a)-lwlib_Z(b)
#define lwlib_CrossVector(c,a,b)	lwlib_X(c) = lwlib_Y(a)*lwlib_Z(b) - lwlib_Z(a)*lwlib_Y(b), \
                				lwlib_Y(c) = lwlib_Z(a)*lwlib_X(b) - lwlib_X(a)*lwlib_Z(b), \
                				lwlib_Z(c) = lwlib_X(a)*lwlib_Y(b) - lwlib_Y(a)*lwlib_X(b)
#define lwlib_DotVector(a,b) (lwlib_X(a)*lwlib_X(b) + lwlib_Y(a)*lwlib_Y(b) + lwlib_Z(a)*lwlib_Z(b))
#define lwlib_ScaleVector(c,a,b) lwlib_X(c)=lwlib_X(a)*(b), lwlib_Y(c)=lwlib_Y(a)*(b), lwlib_Z(c)=lwlib_Z(a)*(b)
#define lwlib_DivideVector(c,a,b) lwlib_X(c)=lwlib_X(a)/(b), lwlib_Y(c)=lwlib_Y(a)/(b), lwlib_Z(c)=lwlib_Z(a)/(b)
#define lwlib_NormalizeVector(n,a) \
	( \
		(n)=sqrt(lwlib_DotVector(a,a)), \
		(n) ? \
			(lwlib_X(a)/=n, lwlib_Y(a)/=n, lwlib_Z(a)/=n) : \
			0 \
	)
#define lwlib_InitVector(v,a,b,c) lwlib_X(v) = (a), lwlib_Y(v) = (b), lwlib_Z(v) = (c)
#define lwlib_InvertVector(a,b) lwlib_X(a) = -lwlib_X(b), lwlib_Y(a) = -lwlib_Y(b), lwlib_Z(a) = -lwlib_Z(b)
#define lwlib_MultVector(c,a,b) lwlib_X(c) = lwlib_X(a) * lwlib_X(b), lwlib_Y(c) = lwlib_Y(a) * lwlib_Y(b), lwlib_Z(c) = lwlib_Z(a) * lwlib_Z(b)
#define lwlib_VectorLength(v) sqrt(lwlib_DotVector(v,v))
#define lwlib_VectorLengthSq(v) lwlib_DotVector(v,v)
#define lwlib_TransformVector(a,b,f) lwlib_X(a) = f(lwlib_X(b)), lwlib_Y(a) = f(lwlib_Y(b)), lwlib_Z(a) = f(lwlib_Z(b))
#define lwlib_PackVector(a) (a)[0], (a)[1], (a)[2]
#define lwlib_UnpackVector(a, b) (b)[0] = lwlib_X(a), (b)[1] = lwlib_Y(a), (b)[2] = lwlib_Z(a)
#define lwlib_ExpandVector(a) lwlib_X(a), lwlib_Y(a), lwlib_Z(a)

#define lwlib_vec3_init(x,y,z) { { (x), (y), (z) } }

#define lwlib_AddVector4(c,a,b) lwlib_X(c)=lwlib_X(a)+lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)+lwlib_Y(b), lwlib_Z(c)=lwlib_Z(a)+lwlib_Z(b), lwlib_W(c)=lwlib_W(a)+lwlib_W(b)
#define lwlib_SubVector4(c,a,b) lwlib_X(c)=lwlib_X(a)-lwlib_X(b), lwlib_Y(c)=lwlib_Y(a)-lwlib_Y(b), lwlib_Z(c)=lwlib_Z(a)-lwlib_Z(b), lwlib_W(c)=lwlib_W(a)-lwlib_W(b)
#define lwlib_DotVector4(a,b) (lwlib_X(a)*lwlib_X(b) + lwlib_Y(a)*lwlib_Y(b) + lwlib_Z(a)*lwlib_Z(b) + lwlib_W(a)*lwlib_W(b))
#define lwlib_ScaleVector4(c,a,b) lwlib_X(c)=lwlib_X(a)*(b), lwlib_Y(c)=lwlib_Y(a)*(b), lwlib_Z(c)=lwlib_Z(a)*(b), lwlib_W(c)=lwlib_W(a)*(b)
#define lwlib_DivideVector4(c,a,b) lwlib_X(c)=lwlib_X(a)/(b), lwlib_Y(c)=lwlib_Y(a)/(b), lwlib_Z(c)=lwlib_Z(a)/(b), lwlib_W(c)=lwlib_W(a)/(b)
#define lwlib_NormalizeVector4(n,a) \
	( \
		(n)=sqrt(lwlib_DotVector4(a,a)), \
		(n) ? \
			(lwlib_X(a)/=n, lwlib_Y(a)/=n, lwlib_Z(a)/=n, lwlib_W(a)/=n) : \
			0 \
	)
#define lwlib_InitVector4(v,a,b,c,d) lwlib_X(v) = (a), lwlib_Y(v) = (b), lwlib_Z(v) = (c), lwlib_W(v) = (d)
#define lwlib_InvertVector4(a,b) lwlib_X(a) = -lwlib_X(b), lwlib_Y(a) = -lwlib_Y(b), lwlib_Z(a) = -lwlib_Z(b), lwlib_W(a) = -lwlib_W(b)
#define lwlib_MultVector4(c,a,b) lwlib_X(c) = lwlib_X(a) * lwlib_X(b), lwlib_Y(c) = lwlib_Y(a) * lwlib_Y(b), lwlib_Z(c) = lwlib_Z(a) * lwlib_Z(b), lwlib_W(c) = lwlib_W(a) * lwlib_W(b)
#define lwlib_VectorLength4(v) sqrt(lwlib_DotVector4(v,v))
#define lwlib_VectorLengthSq4(v) lwlib_DotVector4(v,v)
#define lwlib_TransformVector4(a,b,f) lwlib_X(a) = f(lwlib_X(b)), lwlib_Y(a) = f(lwlib_Y(b)), lwlib_Z(a) = f(lwlib_Z(b)), lwlib_W(a) = f(lwlib_W(b))
#define lwlib_PackVector4(a) (a)[0], (a)[1], (a)[2], (a)[3]
#define lwlib_UnpackVector4(a, b) (b)[0] = lwlib_X(a), (b)[1] = lwlib_Y(a), (b)[2] = lwlib_Z(a), (b)[3] = lwlib_W(a)
#define lwlib_ExpandVector4(a) lwlib_X(a), lwlib_Y(a), lwlib_Z(a), lwlib_W(a)

#define lwlib_vec4_init(x,y,z,w) { { (x), (y), (z), (w) } }

#define lwlib_vec4b_black lwlib_vec4b(0x00, 0x00, 0x00, 0xff)
#define lwlib_vec4b_red lwlib_vec4b(0xff, 0x00, 0x00, 0xff)
#define lwlib_vec4b_green lwlib_vec4b(0x00, 0xff, 0x00, 0xff)
#define lwlib_vec4b_yellow lwlib_vec4b(0xff, 0xff, 0x00, 0xff)
#define lwlib_vec4b_blue lwlib_vec4b(0x00, 0x00, 0xff, 0xff)
#define lwlib_vec4b_cyan lwlib_vec4b(0x00, 0xff, 0xff, 0xff)
#define lwlib_vec4b_magenta lwlib_vec4b(0xff, 0x00, 0xff, 0xff)
#define lwlib_vec4b_white lwlib_vec4b(0xff, 0xff, 0xff, 0xff)

#define lwlib_plane4f lwlib_vec4f_pair

/* access (row,col) component of matrix */
#define lwlib_CM3(a, r, c) (a).v[(r) * 3 + (c)]

/* access (row,col) component of matrix */
#define lwlib_CM4(a, r, c) (a).v[(r) * 4 + (c)]

#define lwlib_RotateArray(arr, n, type)         \
do {                                            \
	int __i;                                    \
	type s;                                     \
                                                \
	assert((n) > 0);                            \
	s = (arr)[0];                               \
                                                \
	for (__i = 0; __i < (n) - 1; __i++)         \
	{                                           \
		(arr)[__i] = (arr)[__i + 1];            \
	}                                           \
                                                \
	(arr)[(n) - 1] = s;                         \
} while(0)

#define lwlib_RotateArrayK(arr, n, k, type)     \
do {                                            \
	int __i;                                    \
	int __k2 = (k);                             \
                                                \
	while (__k2 < 0)                            \
	{                                           \
		__k2 += (n);                            \
	}                                           \
	__k2 %= (n);                                \
                                                \
	for (__i = 0; __i < __k2; __i++)            \
	{                                           \
		lwlib_RotateArray((arr), (n), type);    \
	}                                           \
} while(0)

namespace lwlib
{
	typedef struct BoxData_s
	{
		int faceVertTable[6][4][3];
	} BoxData_t;

	extern BoxData_t g_boxData;

	typedef double (*TFunc)(double x);

	namespace FaceSide
	{
		enum e
		{
			Front,
			Back,
			Split,
			Coplanar,
			Max,
		};
	}

	namespace VectorOp
	{
		enum e
		{
			Add,
			Sub,
			Mult,
			Divide,
			ShiftLeft,
			ShiftRight,
		};
	}

	namespace Facing
	{
		enum
		{
			PositiveX,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ,
			Last = NegativeZ,
		};
	}

	template <typename T, int N>
	class Point
	{
	public:
		T v[N];

		Point()
		{
			reset();
		}

		void reset()
		{
			memset(v, 0, sizeof(v));
		}

		bool operator<(const Point<T, N> &point) const
		{
			int i;
			for (i = 0; i < N; i++)
			{
				if (v[i] < point.v[i])
				{
					return true;
				}
			}

			return false;
		}
	};

	template <typename T, int N>
	std::ostream &operator<<(std::ostream &os, const Point<T, N> &x)
	{
		int i;
		os << std::string("(");
		for (i = 0; i < N; i++)
		{
			os << std::string(" ") << x.v[i];
		}
		os << std::string(" )");
		return os;
	}

	template <typename T>
	class AxialPoint
	{
	public:
		int i;
		T v;

		AxialPoint() : i(0), v(T())
		{
		}

		AxialPoint(int i_, T v_) : i(i_), v(v_)
		{
		}
	};

	typedef Point<double, 2> Point2f;
	typedef Point<double, 3> Point3f;
	typedef Point<double, 4> Point4f;
	typedef Point<int, 2> Point2i;
	typedef Point<int, 3> Point3i;
	typedef Point<int, 4> Point4i;
	typedef Point<unsigned char, 3> Point3b;
	typedef Point<unsigned char, 4> Point4b;

	typedef Point<double, 2> Vector2f;
	typedef Point<double, 3> Vector3f;
	typedef Point<double, 4> Vector4f;
	typedef Point<int, 2> Vector2i;
	typedef Point<int, 3> Vector3i;

	typedef Point<fixedpt, 2> Point2fp;

	typedef AxialPoint<double> AxialPointf;

	template <typename T, int N>
	class PointPair
	{
	public:
		Point<T, N> p[2];

		PointPair()
		{
		}

		PointPair(const Point<T, N> &a, const Point<T, N> &b)
		{
			p[0] = a;
			p[1] = b;
		}
	};

	template <typename T, int N>
	std::ostream &operator<<(std::ostream &os, const PointPair<T, N> &x)
	{
		return os << x.p[0] << " " << x.p[1];
	}

	typedef PointPair<double, 2> PointPair2f;
	typedef PointPair<double, 3> PointPair3f;
	typedef PointPair<double, 4> PointPair4f;
	typedef PointPair<int, 2> PointPair2i;
	typedef PointPair<int, 3> PointPair3i;

	typedef PointPair3f Edge3f;
	typedef PointPair3f Plane3f;
	typedef PointPair3f Ray3f;
	typedef PointPair3f Box3f;

	typedef PointPair4f Plane4f;
	typedef PointPair4f Ray4f;
	typedef PointPair3i Box3i;
	typedef PointPair2i Box2i;
	typedef PointPair2f Box2f;

	typedef PointPair2f Plane2f;

	typedef Point3f (*FacePoint3f_t)(void *priv, int n);

	typedef struct
	{
		void *priv;
		FacePoint3f_t points;
		int numPoints;
	} Face3f_t;

	typedef Plane3f (*RegionPlane3f_t)(void *priv, int n);

	typedef struct
	{
		void *priv;
		RegionPlane3f_t planes;
		int numPlanes;
		int memAllocated;
	} Region3f_t;

	typedef Vector3f (*Func3f_t)(Vector3f v);

	struct Sphere3f
	{
		Point3f origin;
		double radius;
	};

	static inline double lerp(double a, double b, double t)
	{
		return a * (1 - t) + b * t;
	}

	static inline double lerpfp(fixedpt a, fixedpt b, fixedpt t)
	{
		return fixedpt_mul(a, (FIXEDPT_ONE - t)) + fixedpt_mul(b, t);
	}

	static inline int lerpi(int a, int b, int t, int d)
	{
		return (a * (d - t) + b * t) / d;
	}

	static inline double lerp2(double q[4], double u, double v)
	{
		return q[0] * (1-u)*(1-v) + q[1] * (1-v)*u + q[2] * u*v + 
			q[3] * v*(1-u);
	}

	template <typename T, typename R, int N, int M>
	const Point<R, M> vec_convert(const Point<T, N> &a)
	{
		int i;
		Point<R, M> b;

		for (i = 0; i < (N < M ? N : M); i++)
		{
			b.v[i] = (R)a.v[i];
		}

		return b;
	}

	template <typename T, int N>
	Point<T, N> vec_convert(const Point<T, N> &a)
	{
		return a;
	}

	template <typename T>
	Point<T, 2> vec2(T x, T y)
	{
		Point<T, 2> a;
		a.v[0] = x;
		a.v[1] = y;
		return a;
	}

	template <typename T, int N>
	const Point<T, 2> vec2(const Point<T, N> &a)
	{
		return vec2(a.v[0], a.v[1]);
	}

	template <typename T>
	Point<T, 3> vec3(T x, T y, T z)
	{
		Point<T, 3> a;
		a.v[0] = x;
		a.v[1] = y;
		a.v[2] = z;
		return a;
	}

	template <typename T, int N>
	const Point<T, 3> vec3(const Point<T, N> &p)
	{
		return vec_convert<T, T, N, 3>(p);
	}

	template <typename T>
	Point<T, 4> vec4(T x, T y, T z, T w)
	{
		Point<T, 4> a;
		a.v[0] = x;
		a.v[1] = y;
		a.v[2] = z;
		a.v[3] = w;
		return a;
	}

	template <typename T, int N>
	const Point<T, 4> vec4(const Point<T, N> &p)
	{
		return vec_convert<T, T, N, 4>(p);
	}

	template <typename T, int N>
	const Point<int, N> vec_int(const Point<T, N> &a)
	{
		return vec_convert<T, int, N, N>(a);
	}

	template <typename T, int N>
	const Point<double, N> vec_double(const Point<T, N> &a)
	{
		return vec_convert<T, double, N, N>(a);
	}

	template <typename R, typename T, int N>
	const Point<R, N> vec_typeconv(const Point<T, N> &a)
	{
		return vec_convert<T, R, N, N>(a);
	}

	static inline Point2f vec2f(double x, double y)
	{
		return vec2(x, y);
	}

	static inline Point2fp vec2fp(fixedpt x, fixedpt y)
	{
		return vec2(x, y);
	}

	static inline Point2i vec2i(int x, int y)
	{
		return vec2(x, y);
	}

	static inline Point2i vec2i(Point3f p)
	{
		Point<int, 2> a;
		a.v[0] = (int)p.v[0];
		a.v[1] = (int)p.v[1];
		return a;
	}

	static inline Point3i vec3i(int x, int y, int z)
	{
		return vec3(x, y, z);
	}

	static inline Point3f vec3f(double x, double y, double z)
	{
		return vec3(x, y, z);
	}

	static inline Point4f vec4f(double x, double y, double z, double w)
	{
		return vec4(x, y, z, w);
	}

	static inline Point4f vec4f_origin(void)
	{
		return vec4(0.0, 0.0, 0.0, 1.0);
	}

	static inline Point4f vec4f_point(const Point3f &p)
	{
		return vec4(p.v[0], p.v[1], p.v[2], 1.0);
	}

	static inline Point4b vec4b(unsigned char x, unsigned char y, 
		unsigned char z, unsigned char w)
	{
		return vec4(x, y, z, w);
	}

	template <typename T, int N>
	Point<T, N> vec_same(T x)
	{
		int i;
		Point<T, N> a;
		for (i = 0; i < N; i++)
		{
			a.v[i] = x;
		}
		return a;
	}

	static inline Point2i vec2i_same(int a)
	{
		return vec_same<int, 2>(a);
	}

	static inline Point3i vec3i_same(int a)
	{
		return vec_same<int, 3>(a);
	}

	static inline Point3f vec3f_same(double a)
	{
		return vec_same<double, 3>(a);
	}

	template <typename T, int N>
	Point<T, N> vec_zero()
	{
		return vec_same<T, N>(T());
	}

	template <typename T, int N>
	Point<T, N> vec_ones()
	{
		return vec_same<T, N>((T)1);
	}

	template <typename T, int N>
	const Point<T, N> vec_axial(int i, T x)
	{
		Point<T, N> a;
		a.v[i] = x;
		return a;
	}

	template <typename T>
	const Point<T, 2> vec2_axial(int i, T x)
	{
		return vec_axial<T, 2>(i, x);
	}

	template <typename T>
	const Point<T, 3> vec3_axial(int i, T x)
	{
		return vec_axial<T, 3>(i, x);
	}

	template <typename T>
	const Point<T, 4> vec4_axial(int i, T x)
	{
		return vec_axial<T, 4>(i, x);
	}

	static inline Point2f vec2f_zero()
	{
		return vec_zero<double, 2>();
	}

	static inline Point3f vec3f_zero()
	{
		return vec_zero<double, 3>();
	}

	static inline Point3f vec3f_ones()
	{
		return vec_ones<double, 3>();
	}

	static inline Point2i vec2i_zero()
	{
		return vec_zero<int, 2>();
	}

	static inline Point3i vec3i_zero()
	{
		return vec_zero<int, 3>();
	}

	template <typename T, int N>
	const Point<T, N> vec_add(Point<T, N> a, Point<T, N> b)
	{
		int i;
		Point<T, N> c;
		for (i = 0; i < N; i++)
		{
			c.v[i] = a.v[i] + b.v[i];
		}
		return c;
	}

	template <typename T>
	const Point<T, 3> vec_add(Point<T, 3> a, Point<T, 3> b)
	{
		a.v[0] += b.v[0];
		a.v[1] += b.v[1];
		a.v[2] += b.v[2];
		return a;
	}

	template <typename T, int N>
	Point<T, N> operator+(const Point<T, N> &a, const Point<T, N> &b)
	{
		return vec_add(a, b);
	}

	template <typename T, int N>
	Point<T, N> &operator+=(Point<T, N> &a, const Point<T, N> &b)
	{
		a = a + b;
		return a;
	}

	template <typename T, int N>
	const Point<T, N> vec_sub(Point<T, N> a, Point<T, N> b)
	{
		int i;
		Point<T, N> c;
		for (i = 0; i < N; i++)
		{
			c.v[i] = a.v[i] - b.v[i];
		}
		return c;
	}

	template <typename T>
	const Point<T, 3> vec_sub(Point<T, 3> a, Point<T, 3> b)
	{
		a.v[0] -= b.v[0];
		a.v[1] -= b.v[1];
		a.v[2] -= b.v[2];
		return a;
	}

	template <typename T>
	const Point<T, 2> vec_sub(Point<T, 2> a, Point<T, 2> b)
	{
		a.v[0] -= b.v[0];
		a.v[1] -= b.v[1];
		return a;
	}

	template <typename T, int N>
	Point<T, N> operator-(const Point<T, N> &a, const Point<T, N> &b)
	{
		return vec_sub(a, b);
	}

	template <typename T, int N>
	Point<T, N> &operator-=(Point<T, N> &a, const Point<T, N> &b)
	{
		a = a - b;
		return a;
	}

	template <typename T, int N>
	Point<T, N> operator-(const Point<T, N> &a)
	{
		return vec_neg(a);
	}

	template <typename T, int N>
	bool vec_nonzero(const Point<T, N> &a)
	{
		return !vec_equal(a, vec_zero<T, N>(), T());
	}

	template <typename T, int N>
	const Point<T, N> vec_scale(Point<T, N> a, T s)
	{
		int i;
		Point<T, N> b;
		for (i = 0; i < N; i++)
		{
			b.v[i] = a.v[i] * s;
		}
		return b;
	}

	template <typename T>
	const Point<T, 3> vec_scale(Point<T, 3> a, T s)
	{
		a.v[0] *= s;
		a.v[1] *= s;
		a.v[2] *= s;
		return a;
	}

	template <typename T, int N>
	Point<T, N> operator*(T s, const Point<T, N> &a)
	{
		return vec_scale(a, s);
	}

	template <typename T, int N>
	Point<T, N> operator*(const Point<T, N> &a, T s)
	{
		return vec_scale(a, s);
	}

	template <typename T, int N>
	Point<T, N> &operator*=(Point<T, N> &a, T s)
	{
		a = a * s;
		return a;
	}

	template <typename T>
	AxialPoint<T> axialvec_scale(const AxialPoint<T> &a, T s)
	{
		return AxialPoint<T>(a.i, a.v * s);
	}

	template <typename T>
	AxialPoint<T> operator*(T s, const AxialPoint<T> &a)
	{
		return axialvec_scale(a, s);
	}

	template <typename T>
	AxialPoint<T> operator*(const AxialPoint<T> &a, T s)
	{
		return axialvec_scale(a, s);
	}

	template <typename T, int N>
	Point<T, N> operator/(const Point<T, N> &a, T s)
	{
		return vec_divide(a, s);
	}

	template <typename T, int N>
	Point<T, N> &operator/=(Point<T, N> &a, T s)
	{
		a = a / s;
		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_divide(Point<T, N> a, T s)
	{
		int i;
		Point<T, N> b;
		for (i = 0; i < N; i++)
		{
			b.v[i] = a.v[i] / s;
		}
		return b;
	}

	template <typename T, int N>
	Point<T, N> vec_shiftright(Point<T, N> a, T s)
	{
		int i;
		Point<T, N> b;
		for (i = 0; i < N; i++)
		{
			b.v[i] = a.v[i] >> s;
		}
		return b;
	}

	template <typename T, int N>
	Point<T, N> vec_fastmodulo(Point<T, N> a, T s)
	{
		int i;
		Point<T, N> b;
		for (i = 0; i < N; i++)
		{
			b.v[i] = a.v[i] & s;
		}
		return b;
	}

	template <typename T, int N>
	const T vec_dot(Point<T, N> a, Point<T, N> b)
	{
		int i;
		T x = T();
		for (i = 0; i < N; i++)
		{
			x += a.v[i] * b.v[i];
		}
		return x;
	}

	template <typename T>
	const T vec_dot(Point<T, 3> a, Point<T, 3> b)
	{
		return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
	}

	template <typename T>
	const T vec_dot(Point<T, 4> a, Point<T, 4> b)
	{
		return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] +
			a.v[3] * b.v[3];
	}

	template <typename T, int N>
	T vec_dotaxial(const Point<T, N> &a, const AxialPoint<T> &b)
	{
		return a.v[b.i] * b.v;
	}

	template <typename T>
	Point<T, 3> vec3_cross(Point<T, 3> a, Point<T, 3> b)
	{
		Point<T, 3> c;
		lwlib_CrossVector(c, a, b);
		return c;
	}

	template <typename T>
	const T vec2_cross(Point<T, 2> a, Point<T, 2> b)
	{
		return a.v[0] * b.v[1] - a.v[1] * b.v[0];
	}

	template <typename T, int N>
	T vec_length_sq(Point<T, N> a)
	{
		int i;
		T x = T();
		for (i = 0; i < N; i++)
		{
			x += a.v[i] * a.v[i];
		}
		return x;
	}

	template <typename T, int N>
	T vec_length(Point<T, N> a)
	{
		return sqrt(vec_length_sq(a));
	}

	template <typename T, int N>
	T vec_sum(Point<T, N> a)
	{
		int i;
		T x = T();
		for (i = 0; i < N; i++)
		{
			x += a.v[i];
		}
		return x;
	}

	template <typename T, int N>
	T vec_cartesian_length(Point<T, N> a)
	{
		return vec_sum(vec_abs(a));
	}

	template <typename T, int N>
	Point<T, N> vec_normalize(Point<T, N> a)
	{
		T n;

		n = sqrt(vec_dot(a, a));
		if (n != 0.0)
		{
			a = vec_scale(a, 1.0 / n);
		}

		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_trunc_length(Point<T, N> a, double maxlsq)
	{
		double lsq;

		lsq = vec_length_sq(a);
		if (lsq > maxlsq)
		{
			a = vec_normalize(a) * sqrt(maxlsq);
		}

		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_neg(Point<T, N> a)
	{
		int i;
		for (i = 0; i < N; i++)
		{
			a.v[i] = -a.v[i];
		}
		return a;
	}

	template <typename T>
	const Point<T, 2> vec_neg(Point<T, 2> a)
	{
		a.v[0] = -a.v[0];
		a.v[1] = -a.v[1];
		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_mult(Point<T, N> a, Point<T, N> b)
	{
		int i;
		Point<T, N> c;
		for (i = 0; i < N; i++)
		{
			c.v[i] = a.v[i] * b.v[i];
		}
		return c;
	}

	template <typename T, int N>
	T vec_min(Point<T, N> a)
	{
		int i;
		T min = a.v[0];
		for (i = 1; i < N; i++)
		{
			if (a.v[i] < min)
			{
				min = a.v[i];
			}
		}
		return min;
	}

	template <typename T, int N>
	T vec_max(Point<T, N> a)
	{
		int i;
		T max = a.v[0];
		for (i = 1; i < N; i++)
		{
			if (a.v[i] > max)
			{
				max = a.v[i];
			}
		}
		return max;
	}

	template <typename T, int N>
	int vec_maxind(Point<T, N> a)
	{
		int i;
		T max = a.v[0];
		int maxind = 0;
		for (i = 1; i < N; i++)
		{
			if (a.v[i] > max)
			{
				max = a.v[i];
				maxind = i;
			}
		}
		return maxind;
	}

	template <typename T, int N>
	Point<T, N> vec_clip_trunc(Point<T, N> a)
	{
		T max;

		max = vec_max(a);
		if (max > 1.0) 
		{
			a = vec_scale(a, 1.0 / max);
		}
		
		return a;
	}

	template <typename T, int N>
	T vec_range(Point<T, N> a)
	{
		return a.v[1] - a.v[0];
	}

	template <typename T, int N>
	Point<T, N> vec_lerp(Point<T, N> a, Point<T, N> b, T t)
	{
		return vec_add(vec_scale(a, 1 - t), vec_scale(b, t));
	}

	template <typename T, int N>
	const Point<T, N> vec_abs(Point<T, N> a)
	{
		int i;
		for (i = 0; i < N; i++)
		{
			if (a.v[i] < T())
			{
				a.v[i] = -a.v[i];
			}
		}
		return a;
	}

	template <typename T, int N>
	char *vec_to_string(lwlib_String_t string, const char *fmt,
		Point<T, N> a)
	{
		int i;
		lwlib_String_t partStr, fmtStr;

		lwlib_StrCopy(string, "(");
		for (i = 0; i < N; i++)
		{
			lwlib_StrCopy(partStr, string);
			VaString(fmtStr, fmt, a.v[i]);
			VaString(string, "%s%s%s", partStr, fmtStr, 
				(i == N - 1 ? ")" : ","));
		}

		return string;
	}

	template <typename T, int N>
	Point<T, 3> vec_cross(Point<T, N> a, Point<T, N> b)
	{
		return vec3_cross(vec3(a), vec3(b));
	}

	template <typename T, int N>
	Point<T, N> vec_sign(Point<T, N> a)
	{
		int i;
		for (i = 0; i < N; i++)
		{
			a.v[i] = (a.v[i] > 0 ? 1 : (a.v[i] < 0 ? -1 : 0));
		}
		return a;
	}

	template <typename T, int N>
	Point<int, N> vec_sign_int_nonzero(Point<T, N> a)
	{
		int i;
		Point<int, N> b;
		for (i = 0; i < N; i++)
		{
			b.v[i] = (a.v[i] > 0 ? 1 : -1);
		}
		return b;
	}

	static inline Point2i vec2f_int(Point2f a)
	{
		return vec_convert<double, int, 2, 2>(a);
	}

	static inline Point2fp vec2f_fixed(Point2f a)
	{
		return vec2fp(fixedpt_rconst(a.v[0]), 
			fixedpt_rconst(a.v[1]));
	}

	static inline Point2f vec2i_float(Point2i a)
	{
		return vec_convert<int, double, 2, 2>(a);
	}

	static inline Point3i vec3f_int(Point3f a)
	{
		return vec_convert<double, int, 3, 3>(a);
	}

	static inline Point3f vec3i_float(Point3i a)
	{
		return vec_convert<int, double, 3, 3>(a);
	}

	template <typename T, int N>
	Point<T, N> vec_swap(Point<T, N> a)
	{
		int i;
		Point<T, N> b;

		for (i = 0; i < N; i++)
		{
			b.v[i] = a.v[N - i - 1];
		}

		return b;
	}

	template <typename T, int N>
	Point<T, N> vec_transform(Point<T, N> a, T (*f)(T))
	{
		int i;
		for (i = 0; i < N; i++)
		{
			a.v[i] = f(a.v[i]);
		}
		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_floor(Point<T, N> a)
	{
		return vec_transform(a, floor);
	}

	template <typename T, int N>
	Point<int, N> vec_floorint(Point<T, N> a)
	{
		return vec_int(vec_floor(a));
	}

	template <typename T, int N>
	T vec_dist_sq(Point<T, N> a, Point<T, N> b)
	{
		return vec_length_sq(vec_sub(b, a));
	}

	template <typename T, int N>
	T vec_planedot(Point<T, N> a, PointPair<T, N> plane)
	{
		return vec_dot(vec_sub(a, plane.p[0]), plane.p[1]);
	}

	template <typename T, int N>
	Point<T, N> vec_projtoline(Point<T, N> a, PointPair<T, N> ray)
	{
		return ray.p[0] + (vec_dot(a - ray.p[0], ray.p[1]) * ray.p[1]);
	}

	template <typename T, int N>
	T vec_distfromline_sq(Point<T, N> a, PointPair<T, N> ray)
	{
		return vec_length_sq(a - vec_projtoline(a, ray));
	}

	template <typename T, int N>
	int vec_side(PointPair<T, N> plane, Point<T, N> a, T eps)
	{
		const T dot = vec_planedot(a, plane);
		return dot > eps ? 1 : (dot < -eps ? -1 : 0);
	}

	template <typename T, int N>
	Point<T, N> vec_proj(Point<T, N> a, PointPair<T, N> plane)
	{
		const Point<T, N> n = plane.p[1];
		const T dot = vec_planedot(a, plane);
		return vec_sub(a, vec_scale(n, dot));
	}

	template <typename T, int N>
	void vec2_get(const Point<T, N> &a, T &x, T &y)
	{
		x = a.v[0];
		y = a.v[1];
	}

	template <typename T, int N>
	void vec3_get(const Point<T, N> &a, T &x, T &y, T&z)
	{
		x = a.v[0];
		y = a.v[1];
		z = a.v[2];
	}

	template <typename T, int N>
	bool vec_equal(Point<T, N> a, Point<T, N> b, T eps)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (a.v[i] - b.v[i] > eps || a.v[i] - b.v[i] < -eps)
			{
				return false;
			}
		}

		return true;
	}

	template <typename T, int N>
	const Point<T, N> vec_zero_component(Point<T, N> a, int i)
	{
		a.v[i] = T();
		return a;
	}

	template <typename T, int N>
	const Point<T, N> vec_neg_component(Point<T, N> a, int i)
	{
		a.v[i] = -a.v[i];
		return a;
	}

	template <typename T, int N>
	bool operator==(const Point<T, N> &a, const Point<T, N> &b)
	{
		return vec_equal(a, b, T());
	}

	template <typename T, int N>
	bool operator!=(const Point<T, N> &a, const Point<T, N> &b)
	{
		return !vec_equal(a, b, T());
	}

	template <typename T, int N>
	int vec_sign_bits(Point<T, N> a, T b)
	{
		int i;
		int bits = 0;

		for (i = 0; i < N; i++)
		{
			if (a.v[i] >= b)
			{
				bits |= (1 << i);
			}
		}
		
		return bits;
	}

	template <typename T, int N>
	Point<T, N> vec_reflect(Point<T, N> v, Point<T, N> n)
	{
		return vec_sub(vec_scale(n, 2 * vec_dot(v, n)), v);
	}

	template <typename T, int N>
	bool vec_check_pow2(Point<T, N> a)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (!lwlib_IntegerIsPow2((int)a.v[i]))
			{
				return false;
			}
		}

		return true;
	}


	template <typename T, int N>
	Point<T, N> vec_rot90(Point<T, N> a)
	{
		lwlib_Swap(a.v[0], a.v[1], T);
		a.v[0] = -a.v[0];
		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_rot_quadrant(Point<T, N> a, int quadrant)
	{
		for (int i = 0; i < quadrant; i++)
		{
			a = vec_rot90(a);
		}
		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_from_bits(int bits)
	{
		int i;
		Point<T, N> a = vec_zero<T, N>();

		for (i = 0; i < N; i++)
		{
			if (bits & (1 << i))
			{
				a.v[i] = (T)1;
			}
		}

		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_box_inc(Point<T, N> a, Point<T, N> b, Point<T, N> c)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (c.v[i] < b.v[i] - 1)
			{
				c.v[i]++;
				break;
			}
			else // c.v[i] == b.v[i] - 1
			{
				c.v[i] = a.v[i];
			}
		}

		return c;
	}

	template <typename T, int N>
	Point<T, N> vec_bounded_inc(Point<T, N> a, Point<T, N> b)
	{
		return vec_box_inc(vec_zero<T, N>(), b, a);
	}

	template <typename T, int N>
	Point<T, N> vec_from_string(const char *string, const char *fmt)
	{
		int i;
		lwlib_String_t tmpstr;
		Point<T, N> pos;
		char *p, *q;
		char ch;
		int state;

		i = 0;
		lwlib_StrCopy(tmpstr, string);
		pos = vec_zero<T, N>();
		p = tmpstr;
		state = 0;

		while (1)
		{
			ch = *p;
			if (isspace(ch))
			{
				continue;
			}
			if (ch == '\0')
			{
				return pos;
			}

			switch (state)
			{
			case 0:
				if (ch == '(')
				{
					state = 1;
				}
				else
				{
					return pos;
				}
				break;
			case 1:
				if (ch == '-' || isdigit(ch))
				{
					q = p;
					state = 2;
				}
				else
				{
					return pos;
				}
				break;
			case 2:
				if (isdigit(ch))
				{
					// no action
				}
				else if (ch == ',' || ch == ')')
				{
					*p = '\0';
					sscanf(q, fmt, &pos.v[i]);
					i++;

					if (ch == ',')
					{
						if (i < N)
						{
							state = 1;
						}
						else
						{
							return pos;
						}
					}
					else if (ch == ')')
					{
						return pos;
					}
				}
				else
				{
					return pos;
				}
				break;
			}
			p++;
		}

		return pos;
	}

	template <typename T, int N>
	Point<T, N> vec_facing_normal(int facing)
	{
		Point<T, N> normal = vec_zero<T, N>();
		if (facing < 0)
		{
			facing = -(facing + 1);
		}
		normal.v[facing >> 1] = (T)lwlib_OneMinusNx2(facing & 1);
		return normal;
	}

	template <typename T, int N>
	Point<T, N> vec_clip_minmax(Point<T, N> a, Point<T, N> b, Point<T, N> c)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			a.v[i] = lwlib_CLIP(a.v[i], b.v[i], c.v[i]);
		}

		return a;
	}

	template <typename T, int N>
	Point<T, N> vec_op_scalar(Point<T, N> a, VectorOp::e op, int b)
	{
#define OP_SCALAR(vectorOp, binaryOp) \
	case VectorOp::vectorOp: \
		for (i = 0; i < N; i++) \
		{ \
			a.v[i] = a.v[i] binaryOp b; \
		} \
		break;

		int i;
		switch (op)
		{
			OP_SCALAR(Add, +)
			OP_SCALAR(Sub, -)
			OP_SCALAR(Mult, *)
			OP_SCALAR(Divide, /)
			OP_SCALAR(ShiftLeft, <<)
			OP_SCALAR(ShiftRight, >>)
			default:
				assert(0);
		}
		return a;
#undef OP_SCALAR
	}

	template <typename T, int N>
	bool vec_hit_plane(const Point<T, N> &start, const Point<T, N> &end,
		const PointPair<T, N> &plane, bool allowRear, 
		Point<T, N> *hit, double *tval)
	{
		T dist;
		Point<T, N> v;
		T dirProj;
		double t;
		Point<double, N> startf, endf, hitf;

		dist = 
			vec_dot(
				vec_sub(start, plane.p[0]), 
				plane.p[1]
				);

		v = vec_sub(end, start);
		dirProj = vec_dot(v, plane.p[1]);

		if ((dist > 0 && dirProj < 0) || (allowRear && (dist < 0 && dirProj > 0)))
		{
			if (hit != NULL)
			{
				t = -dist / dirProj;
				if (tval != NULL)
				{
					*tval = t;
				}

				startf = vec_convert<T, double, N, N>(start);
				endf = vec_convert<T, double, N, N>(end);
				hitf = vec_lerp(startf, endf, t);

				*hit = vec_convert<double, T, N, N>(hitf);
			}

			return true;
		}

		return false;
	}

	template <typename T, int N>
	PointPair<T, N> box_plane_bynum(const PointPair<T, N> &box,
		int planeNum)
	{
		PointPair<T, N> plane;

		plane.p[0] = box.p[planeNum & 1];

		plane.p[1] = vec_zero<T, N>();
		plane.p[1].v[planeNum / 2] = (planeNum & 1) ? 1 : -1;

		return plane;
	}

	template <typename T, int N>
	Point<T, N> vec_hit_box(const Point<T, N> &start,
		const Point<T, N> &end, const PointPair<T, N> &box, 
		int &planeHitNum, double &tval)
	{
		int i;
		double tmax;
		Point<T, N> finalHit;

		finalHit = end;
		tmax = 0.0;

		for (i = 0; i < N * 2; i++)
		{
			PointPair<T, N> plane = box_plane_bynum(box, i);

			Point<T, N> hit;
			double t;

			if (vec_hit_plane(start, end, plane, false, &hit, &t) && 
				t > tmax)
			{
				tval = tmax = t;
				finalHit = hit;
				planeHitNum = i;
			}
		}

		return finalHit;
	}

	template <typename T, int N>
	Point<T, N> vec_hit_box(const Point<T, N> &start, 
		const Point<T, N> &end, const PointPair<T, N> &box,
		double &tval)
	{
		int n = -1;
		return vec_hit_box(start, end, box, n, tval);
	}

	template <typename T, int N>
	Point<T, N> vec_hit_box(const Point<T, N> &start, 
		const Point<T, N> &end, const PointPair<T, N> &box)
	{
		int n = -1;
		double tval = -1;
		return vec_hit_box(start, end, box, tval);
	}

	template <int N>
	bool vec_line_hits_box(const lwlib::Point<double, N> &start,
		const lwlib::Point<double, N> &end,
		const lwlib::PointPair<double, N> &box)
	{
		double t = -1.0;
		const lwlib::Point<double, N> hit =
			vec_hit_box(start, end, box, t);
		const lwlib::Point<double, N> boxCentre =
			(box.p[0] + box.p[1]) * 0.5;
		return t >= 0.0 && t <= 1.0 &&
			vec_max(vec_abs(hit - boxCentre)) < 0.5 + 0.01;
	}

	template <typename T, int N>
	PointPair<T, N> vec_pair(Point<T, N> a, Point<T, N> b)
	{
		PointPair<T, N> pair;
		pair.p[0] = a;
		pair.p[1] = b;
		return pair;
	}

	static inline Ray3f ray3f(Point3f a, Point3f b)
	{
		return vec_pair<double, 3>(a, b);
	}

	static inline Box2i box2i(Point2i a, Point2i b)
	{
		return vec_pair<int, 2>(a, b);
	}

	static inline Box2i box2i(int x, int y, int w, int h)
	{
		return box2i(vec2i(x, y), vec2i(x + w, y + h));
	}

	static inline Box3i box3i(Point3i a, Point3i b)
	{
		return vec_pair<int, 3>(a, b);
	}

	template <typename T, int N>
	PointPair<T, N> vec_pair_reverse(PointPair<T, N> a)
	{
		PointPair<T, N> b;
		b.p[0] = a.p[1];
		b.p[1] = a.p[0];
		return b;
	}

	template <typename T, int N>
	bool vec_pair_equal(PointPair<T, N> a, PointPair<T, N> b, T eps)
	{
		return vec_equal(a.p[0], b.p[0], eps) &&
			vec_equal(a.p[1], b.p[1], eps);
	}

	template <typename T, int N>
	bool vec_pair_reverse_equal(PointPair<T, N> a, PointPair<T, N> b, T eps)
	{
		return vec_pair_equal(vec_pair_reverse(a), b, eps);
	}

	template <typename T, int N>
	Point<T, N> vec_pair_edge(PointPair<T, N> a)
	{
		return vec_sub(a.p[1], a.p[0]);
	}

	static inline double normalize_angle(double angle)
	{
		while (angle < 0)
		{
			angle += 360.0;
		}
		while (angle >= 360.0)
		{
			angle -= 360.0;
		}

		return angle;
	}

	static inline double normalize_angle180(double angle)
	{
		while (angle < -180.0)
		{
			angle += 360.0;
		}
		while (angle >= 180.0)
		{
			angle -= 360.0;
		}

		return angle;
	}

    static inline double smallest_angle_bw(double sourceA, double targetA)
    {
        sourceA = normalize_angle180(sourceA);
        targetA = normalize_angle180(targetA);

        double a = targetA - sourceA;
        a += (a>180) ? -360 : (a<-180) ? 360 : 0;
        return a;
    }

	template <typename T, int N>
	double vec_line_angle(const Point<T, N> &a, const Point<T, N> &b)
	{
		double angle;
		Point<T, N> c;
		
		c = b - a;
		angle = lwlib_RadToDeg(atan((double)c.v[1] / (double)c.v[0]));

		if (b.v[0] < a.v[0])
		{
			angle += 180.0;
		}

		return normalize_angle(angle);
	}

	template <typename T, int N>
	double vec_angleto(const Point<T, N> &a, double angle, 
		const Point<T, N> &b)
	{
		Point<T, N> dir;
		dir.v[0] = (T)1;
		dir = vec_rot(dir, angle, 2);

		return lwlib_RadToDeg(
			acos(
				vec_dot(b - a, dir) / vec_length(b - a)
				)
			);
	}

	template <typename T, int N>
	T vec_anglebw(const Point<T, N> &a, const Point<T, N> &b)
	{
		return normalize_angle(
			asin(vec_length(vec_cross(a, b))) * 180 / M_PI);
	}

	template <typename T, int N>
	double vec_angle(const Point<T, N> &a)
	{
		if (a.v[0] == 0)
		{
			return 0;
		}

		double angle = atan(a.v[1] / a.v[0]) * 180 / M_PI;
		if (a.v[0] < 0)
		{
			angle += 180.0;
		}

		return normalize_angle(angle);
	}

	template <typename T, int N>
	AxialPoint<T> vec_getaxial(const Point<T, N> &a)
	{
		AxialPoint<T> b;
		for (int i = 0; i < N; i++)
		{
			if (a.v[i] != T())
			{
				b.i = i;
				b.v = a.v[i];
				break;
			}
		}
		return b;
	}

	static inline Edge3f edge3f_reverse(Edge3f a)
	{
		return vec_pair_reverse(a);
	}

	static inline bool edge3f_adj(Edge3f a, Edge3f b, double eps)
	{
		return vec_pair_reverse_equal(a, b, eps);
	}

	static inline Vector3f edge3f_vec(Edge3f a)
	{
		return vec_pair_edge(a);
	}

	static inline Point3f edge3f_lerp(Edge3f a, double t)
	{
		return vec_add(vec_scale(a.p[0], 1 - t), vec_scale(a.p[1], t));
	}

	static inline Plane2f plane2f(Point2f a, Point2f b)
	{
		return vec_pair<double, 2>(a, b);
	}

	static inline Plane3f plane3f(Point3f a, Point3f b)
	{
		return vec_pair<double, 3>(a, b);
	}

	static inline Box2f box2f(Point2f a, Point2f b)
	{
		return vec_pair<double, 2>(a, b);
	}

	static inline Box3f box3f(Point3f a, Point3f b)
	{
		return vec_pair<double, 3>(a, b);
	}

	extern unsigned long vec3f_quantize(Point3f a);
	extern Vector3f vec3f_normal(Point3f a, Point3f b, Point3f c);
	extern Plane3f vec3f_plane(Point3f a, Point3f b, Point3f c);
	extern int vec3f_sphere_side(Sphere3f a, Point3f b, double eps);
	extern double vec3f_face_area(Face3f_t face, Vector3f normal);
	extern Face3f_t vec3f_face(void *priv, FacePoint3f_t points, 
		int numPoints);
	extern int vec3f_face_side(Face3f_t face, Plane3f plane, double eps);
	extern Box3f vec3f_face_box(Face3f_t face);
	extern Point3f vec3f_face_sum(Face3f_t face);
	extern Point3f vec3f_face_center(Face3f_t face);
	extern Point3f vec3f_face_point(void *priv, int n);

	extern Point3f vec3f_quad_lerp(Point3f quad[4], double u, double v);
	extern Point3f vec3f_rot90(Point3f a);
	extern Point3f vec3f_rot90_anchor(Point3f a, Point3f b);

	/* matrix math */

	template <typename T, int N>
	class Mat
	{ 
	public:
		T v[N * N];
		int rows, cols;

		Mat() :
			rows(N), cols(N)
		{
			memset(v, 0, sizeof(v));
		}

		T &at(int i, int j)
		{
			return v[i * N + j];
		}

		const T &at(int i, int j) const
		{
			return v[i * N + j];
		}

		const Point<T, N> col(int j) const
		{
			Point<T, N> a;
			for (int i = 0; i < rows; i++)
			{
				a.v[i] = at(i, j);
			}
			return a;
		}

		const Point<T, N> row(int i) const
		{
			Point<T, N> a;
			for (int j = 0; j < cols; j++)
			{
				a.v[j] = at(i, j);
			}
			return a;
		}
	};

	typedef Mat<double, 2> Mat2f;
	typedef Mat<double, 3> Mat3f;
	typedef Mat<double, 4> Mat4f;

	template <typename T, int N>
	Mat<T, N> mat_zero(int rows, int cols)
	{
		int i;
		Mat<T, N> m;

		for (i = 0; i < N * N; i++)
		{
			m.v[i] = T();
		}
		m.rows = rows;
		m.cols = cols;

		return m;
	}

	template <typename T, int N>
	Mat<T, N> mat_ident(int rows, int cols)
	{
		int i;
		Mat<T, N> m;

		m = mat_zero<T, N>(rows, cols);

		for (i = 0; i < N; i++)
		{
			m.v[i * N + i] = (T)1;
		}
		m.rows = rows;
		m.cols = cols;

		return m;
	}

	static inline Mat2f mat2f_ident(int rows, int cols)
	{
		return mat_ident<double, 2>(rows, cols);
	}

	static inline Mat3f mat3f_ident(int rows, int cols)
	{
		return mat_ident<double, 3>(rows, cols);
	}

	static inline Mat4f mat4f_ident(int rows, int cols)
	{
		return mat_ident<double, 4>(rows, cols);
	}

	template <typename T, int N>
	Mat<T, N> mat_row_mat(int cols, Point<T, N> a)
	{
		int i;
		Mat<T, N> m;

		m = mat_zero<T, N>(1, cols);
		for (i = 0; i < N; i++)
		{
			m.v[i] = a.v[i];
		}
		
		return m;
	}

	template <typename T, int N>
	Mat<T, N> mat_col_mat(int rows, Point<T, N> a)
	{
		int i;
		Mat<T, N> m;

		m = mat_zero<T, N>(rows, 1);
		for (i = 0; i < N; i++)
		{
			m.v[i * N] = a.v[i];
		}
		
		return m;
	}

	template <typename T, int N>
	Mat<T, N> mat_diag(int rows, int cols, Point<T, N> a)
	{
		int i;
		Mat<T, N> m;

		m = mat_zero<T, N>(rows, cols);
		for (i = 0; i < N; i++)
		{
			m.v[i * N + i] = a.v[i];
		}
		
		return m;
	}

	template <typename T, int N>
	Mat<T, N> mat_shift(int axis)
	{
		assert(axis >= 0 && axis < N);
		return mat_rotate_columns(mat_ident<T, N>(N, N), N - 1 - axis);
	}

	template <typename T, typename R, int N, int M>
	void mat_convert(const Mat<T, N> &a, Mat<R, M> &b)
	{
		const int minN = (N < M ? N : M);
		for (int i = 0; i < minN; i++)
		{
			for (int j = 0; j < minN; j++)
			{
				b.at(i, j) = a.at(i, j);
			}
		}
	}

	template <typename T>
	const Mat<T, 3> mat_cross(const Point<T, 3> &a)
	{
		Mat<T, 3> m;
		m.at(0, 1) = -a.v[2];
		m.at(0, 2) = a.v[1];
		m.at(1, 2) = -a.v[0];
		m.at(1, 0) = a.v[2];
		m.at(2, 0) = -a.v[1];
		m.at(2, 1) = a.v[0];
		return m;
	}

	template <typename T, int N>
	const Mat<T, N> mat_tensor(const Point<T, N> &a)
	{
		Mat<T, N> m;
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				m.at(i, j) = a.v[i] * a.v[j];
			}
		}
		return m;
	}

	template <typename T, int N>
	Mat<T, N> mat_rot(double angle, int axis)
	{
		double rad;
		double sinv, cosv;
		Mat<T, 3> mat = mat_ident<T, 3>(3, 3);
		Mat<T, 3> shift = mat_shift<T, 3>(axis);

		while (angle < 0)
			angle += 360;
		while (angle >= 360)
			angle -= 360;
		
		if (angle == 0)
		{
			cosv = 1;
			sinv = 0;
		}
		else if (angle == 90)
		{
			cosv = 0;
			sinv = 1;
		}
		else if (angle == 180)
		{
			cosv = -1;
			sinv = 0;
		}
		else if (angle == 270)
		{
			cosv = 0;
			sinv = -1;
		}
		else
		{
			rad = angle / 180 * M_PI;
			cosv = cos(rad);
			sinv = sin(rad);
		}

		mat.v[0] = cosv;
		mat.v[1] = -sinv;
		mat.v[3 + 0] = sinv;
		mat.v[3 + 1] = cosv;
		
		Mat<T, N> res = mat_ident<T, N>(N, N);
		mat = mat_mult(mat_mult(mat_xpose(shift), mat), shift);
		mat_convert(mat, res);
		return res;
	}

	template <typename T, int N>
	const Mat<T, N> mat_rot(T angle, const Point<T, N> &u)
	{
		T rad, sinv, cosv;

		angle = normalize_angle(angle);
		
		if (angle == 0)
		{
			cosv = 1;
			sinv = 0;
		}
		else if (angle == 90)
		{
			cosv = 0;
			sinv = 1;
		}
		else if (angle == 180)
		{
			cosv = -1;
			sinv = 0;
		}
		else if (angle == 270)
		{
			cosv = 0;
			sinv = -1;
		}
		else
		{
			rad = angle / 180 * M_PI;
			cosv = cos(rad);
			sinv = sin(rad);
		}

		const Point<T, 3> u3 = vec3(u);
		const Mat<T, 3> I = mat_ident<T, 3>(3, 3);
		const Mat<T, 3> R = (I * cosv) + (mat_cross(u3) * sinv) +
			(mat_tensor(u3) * (1 - cosv));

		Mat<T, N> res = mat_ident<T, N>(N, N);
		mat_convert(R, res);
		return res;
	}

	template <typename T, int N>
	Mat<T, N> mat_translate(const Point<T, N> &point)
	{
		Mat<T, N> xlateMat = mat_ident<T, N>(N, N);

		for (int i = 0; i < N - 1; i++)
		{
			xlateMat.at(i, N - 1) = point.v[i];
		}
		xlateMat.at(N - 1, N - 1) = 1.0;

		return xlateMat;
	}

	template <typename T, int N>
	const Mat<T, N> mat_negcpt(int i)
	{
		Mat<T, N> m = mat_ident<T, N>(N, N);
		m.at(i, i) = -1;
		return m;
	}

	template <typename T, int N>
	Point<T, N> vec_rot(const Point<T, N> &a, double angle, int axis)
	{
		return mat_xform(mat_rot<T, N>(angle, axis), a);
	}

	static inline Mat2f mat2f_rot(double angle)
	{
		double rad;
		double sinv, cosv;
		Mat2f mat = mat2f_ident(2, 2);

		while (angle < 0)
			angle += 360;
		while (angle >= 360)
			angle -= 360;
		
		if (angle == 0)
		{
			cosv = 1;
			sinv = 0;
		}
		else if (angle == 90)
		{
			cosv = 0;
			sinv = 1;
		}
		else if (angle == 180)
		{
			cosv = -1;
			sinv = 0;
		}
		else if (angle == 270)
		{
			cosv = 0;
			sinv = -1;
		}
		else
		{
			rad = angle / 180 * M_PI;
			cosv = cos(rad);
			sinv = sin(rad);
		}

		mat.v[0] = cosv;
		mat.v[1] = -sinv;
		mat.v[2] = sinv;
		mat.v[3] = cosv;

        return mat;
	}

	static inline Mat3f mat3f_rot(double angle, int axis)
	{
		return mat_rot<double, 3>(angle, axis);
	}

	template <typename T, int N>
	Mat<T, N> mat_add(Mat<T, N> a, Mat<T, N> b)
	{
		int i;
		assert(a.rows == b.rows && a.cols == b.cols);

		for (i = 0; i < N * N; i++)
		{
			a.v[i] += b.v[i];
		}

		return a;
	}

	template <typename T, int N>
	const Mat<T, N> operator+(const Mat<T, N> &a, const Mat<T, N> &b)
	{
		return mat_add(a, b);
	}

	template <typename T, int N>
	Mat<T, N> mat_sub(Mat<T, N> a, Mat<T, N> b)
	{
		int i;
		assert(a.rows == b.rows && a.cols == b.cols);

		for (i = 0; i < N * N; i++)
		{
			a.v[i] -= b.v[i];
		}

		return a;
	}

	template <typename T, int N>
	const Mat<T, N> operator-(const Mat<T, N> &a, const Mat<T, N> &b)
	{
		return mat_sub(a, b);
	}

	template <typename T, int N>
	Mat<T, N> mat_mult(Mat<T, N> a, Mat<T, N> b)
	{
		int i, j, k;
		Mat<T, N> c;

		assert(a.cols == b.rows);
		
		c = mat_zero<T, N>(a.rows, b.cols);
		for (i = 0; i < a.rows; i++)
		{
			for (j = 0; j < b.cols; j++)
			{
				for (k = 0; k < a.cols; k++)
				{
					c.v[i * N + j] += a.v[i * N + k] * b.v[k * N + j];
				}
			}
		}

		return c;
	}

	template <typename T, int N>
	Mat<T, N> mat_xpose(Mat<T, N> a)
	{
		int i, j;
		Mat<T, N> b;

		b = mat_zero<T, N>(a.cols, a.rows);
		for (i = 0; i < a.cols; i++)
		{
			for (j = 0; j < a.rows; j++)
			{
				b.v[i * N + j] = a.v[j * N + i];
			}
		}

		return b;
	}

	template <typename T, int N>
	const Mat<T, N> mat_scale(Mat<T, N> a, T b)
	{
		int i, j;
		for (i = 0; i < a.rows; i++)
		{
			for (j = 0; j < a.cols; j++)
			{
				a.v[i * N + j] *= b;
			}
		}
		return a;
	}

	template <typename T, int N>
	const Mat<T, N> operator*(const Mat<T, N> &a, T b)
	{
		return mat_scale(a, b);
	}

	template <typename T, int N>
	Mat<T, N> mat_rotate_columns(Mat<T, N> a, int times)
	{
		int i;

		for (i = 0; i < a.rows; i++)
		{
			lwlib_RotateArrayK(&a.v[i * N + 0], a.cols, times, T);
		}

		return a;
	}

	template <typename T, int N>
	Point<T, N> mat_row(Mat<T, N> a, int row)
	{
		int i;
		Point<T, N> p;
		assert(row < a.rows);

		for (i = 0; i < N; i++)
		{
			p.v[i] = a.v[row * N + i];
		}

		return p;
	}

	template <typename T, int N>
	Point<T, N> mat_col(Mat<T, N> a, int col)
	{
		int i;
		Point<T, N> p;
		assert(col < a.cols);

		for (i = 0; i < N; i++)
		{
			p.v[i] = a.v[i * N + col];
		}

		return p;
	}

	template <typename T, int N>
	T &mat_elem(Mat<T, N> &a, int row, int col)
	{
        return a.v[row * N + col];
	}

	template <typename T, int N>
	const T &mat_elem(const Mat<T, N> &a, int row, int col)
	{
        return a.v[row * N + col];
	}

	template <typename T, int N>
	const T mat_elem_neigh(const Mat<T, N> &a, int row, int col, int c)
	{
		int neighRow, neighCol;

		neighRow = row - 1 + (c / 3);
		neighCol = col - 1 + (c % 3);

		if (neighRow < 0 || neighRow >= a.rows || 
			neighCol < 0 || neighCol >= a.cols)
		{
			return T();
		}

		return mat_elem(a, neighRow, neighCol);
    }

	template <typename T, int N>
	const Point<T, N> mat_xform(const Mat<T, N> &a, const Point<T, N> &b)
	{
		int i, j;
		Point<T, N> c;

		for (i = 0; i < N; i++)
		{
			c.v[i] = T();
			for (j = 0; j < N; j++)
			{
				c.v[i] += a.v[i * N + j] * b.v[j];
			}
		}

		return c;
	}

	template <typename T>
	const Point<T, 4> mat_xform(const Mat<T, 4> &a, const Point<T, 4> &b)
	{
		Point<T, 4> c;
		c.v[0] = a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] +
			a.v[3] * b.v[3];
		c.v[1] = a.v[4] * b.v[0] + a.v[5] * b.v[1] + a.v[6] * b.v[2] +
			a.v[7] * b.v[3];
		c.v[2] = a.v[8] * b.v[0] + a.v[9] * b.v[1] + a.v[10] * b.v[2] +
			a.v[11] * b.v[3];
		c.v[3] = a.v[12] * b.v[0] + a.v[13] * b.v[1] + a.v[14] * b.v[2] +
			a.v[15] * b.v[3];
		return c;
	}

	template <typename T, int N>
	const PointPair<T, N> mat_xform_pair(const Mat<T, N> &a,
		const PointPair<T, N> &b)
	{
		PointPair<T, N> pair;
		pair.p[0] = mat_xform(a, b.p[0]);
		pair.p[1] = mat_xform(a, b.p[1]);
		return pair;
	}

	template <typename T, int N>
	Mat<T, N> mat_lerp(Mat<T, N> a[4], T u, T v)
	{
		int i, j, k;
		Mat<T, N> b;
		T q[4];

		b = mat_zero<T, N>(a[0].rows, a[0].cols);

		for (i = 0; i < b.rows; i++)
		{
			for (j = 0; j < b.cols; j++)
			{
				for (k = 0; k < 4; k++)
				{
					q[k] = mat_elem(a[k], i, j);
				}
				mat_elem(b, i, j) = lerp2(q, u, v);
			}
		}

		return b;
	}

	static inline Plane3f mat3f_rotate_plane(Mat3f a, Plane3f b)
	{
		return mat_xform_pair(a, b);
	}

	template <typename T, int N>
	char *mat_to_string(lwlib_String_t string, const char *fmt,
		const Mat<T, N> &a)
	{
		int row, col;
		lwlib_String_t partStr, valStr;

		lwlib_StrCopy(string, "(\n");
		for (row = 0; row < a.rows; row++)
		{
            for (col = 0; col < a.cols; col++)
            {
                lwlib_StrCopy(partStr, string);
                lwlib_VaString(valStr, fmt, mat_elem(a, row, col));
                lwlib_VaString(string, "%s%s, ", partStr, valStr);
            }
            lwlib_StrCopy(partStr, string);
            lwlib_VaString(string, "%s\n", partStr);
		}
        lwlib_StrCopy(partStr, string);
        lwlib_VaString(string, "%s)\n", partStr);

		return string;
	}

	class Xform
	{
	public:
		Mat4f mat;
		Mat4f invMat;

		Xform() :
			mat(mat4f_ident(4, 4)),
			invMat(mat4f_ident(4, 4))
		{
		}

		Xform(Mat4f mat_, Mat4f invMat_) :
			mat(mat_),
			invMat(invMat_)
		{
		}

		Xform mult(const Xform &xform) const
		{
			return Xform(
				mat_mult(mat, xform.mat),
				mat_mult(xform.invMat, invMat)
				);
		}

		const Xform operator*(const Xform &xform) const
		{
			return mult(xform);
		}

		const Point4f apply(Point4f p) const
		{
			return mat_xform(mat, p);
		}

		const Point4f operator*(Point4f p) const
		{
			return apply(p);
		}

		Plane4f apply(Plane4f plane) const
		{
			return vec_pair(apply(plane.p[0]), apply(plane.p[1]));
		}

		const Plane4f operator*(Plane4f plane) const
		{
			return apply(plane);
		}

		Xform invert(void) const
		{
			return Xform(invMat, mat);
		}
	};

	static inline Xform xform_translate(Vector4f disp)
	{
		return Xform(mat_translate(disp), mat_translate(-disp));
	}

	static inline Xform xform_rot(double angle, int axis)
	{
		return Xform(mat_rot<double, 4>(angle, axis), 
			mat_rot<double, 4>(-angle, axis));
	}

	static inline Xform xform_rot(double angle, const Point4f &u)
	{
		return Xform(mat_rot<double, 4>(angle, u), 
			mat_rot<double, 4>(-angle, u));
	}

	static inline Xform xform_negcpt(int i)
	{
		const Mat4f m = mat_negcpt<double, 4>(i);
		return Xform(m, m);
	}

	template <typename T, int N>
	PointPair<T, N> box_cell(Point<T, N> a)
	{
		return vec_pair(a, vec_add(a, vec_ones<T, N>()));
	}

	template <typename T, int N>
	PointPair<T, N> box_combine(PointPair<T, N> a, PointPair<T, N> b)
	{
		int i;
		PointPair<T, N> c;

		for (i = 0; i < N; i++)
		{
			c.p[0].v[i] = std::min(a.p[0].v[i], b.p[0].v[i]);
			c.p[1].v[i] = std::max(a.p[1].v[i], b.p[1].v[i]);
		}

		return c;
	}

	template <typename T, int N>
	PointPair<T, N> box_expand(PointPair<T, N> a, Point<T, N> b)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			a.p[0].v[i] = std::min(a.p[0].v[i], b.v[i]);
			a.p[1].v[i] = std::max(a.p[1].v[i], b.v[i]);
		}

		return a;
	}

	template <typename T, int N>
	PointPair<T, N> box_grow(PointPair<T, N> a, T size)
	{
		PointPair<T, N> pair;
		pair.p[0] = vec_sub(a.p[0], vec_same<T, N>(size));
		pair.p[1] = vec_add(a.p[1], vec_same<T, N>(size));
		return pair;
	}

	template <typename T, int N>
	PointPair<T, N> box_grow(PointPair<T, N> a, Point<T, N> b)
	{
		PointPair<T, N> pair;
		pair.p[0] = vec_sub(a.p[0], b);
		pair.p[1] = vec_add(a.p[1], b);
		return pair;
	}

	template <typename T, int N>
	bool box_touch(PointPair<T, N> a, PointPair<T, N> b)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (a.p[0].v[i] >= b.p[1].v[i] ||
				a.p[1].v[i] <= b.p[0].v[i])
			{
				return false;
			}
		}

		return true;
	}

	template <typename T, int N>
	PointPair<T, N> box_intersect(PointPair<T, N> a, PointPair<T, N> b)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			a.p[0].v[i] = std::max(a.p[0].v[i], b.p[0].v[i]);
			a.p[1].v[i] = std::min(a.p[1].v[i], b.p[1].v[i]);
		}

		return a;
	}

	template <typename T, int N>
	PointPair<T, N> box_add(PointPair<T, N> a, Point<T, N> b)
	{
		int i;

		for (i = 0; i < 2; i++)
		{
			a.p[i] = vec_add(a.p[i], b);
		}

		return a;
	}

	template <typename T, int N>
	PointPair<T, N> box_sub(PointPair<T, N> a, Point<T, N> b)
	{
		int i;

		for (i = 0; i < 2; i++)
		{
			a.p[i] = vec_sub(a.p[i], b);
		}

		return a;
	}

	template <typename T>
	Point<T, 3> box3_face_vert(const PointPair<T, 3> box, int facing, 
		int vertNum)
	{
		const int *row;
		assert(facing >= -6 && facing < 6 && vertNum >= 0 && vertNum < 4);

		if (facing >= 0)
		{
			row = g_boxData.faceVertTable[facing][vertNum];
		}
		else
		{
			facing = -(facing + 1);
			facing += lwlib_OneMinusNx2(facing & 1);
			row = g_boxData.faceVertTable[facing][3 - vertNum];
		}

		Point<T, 3> vert;
		lwlib_RepeatN(3, i, 
			vert.v[i] = box.p[row[i]].v[i]);
		return vert;
	}

	template <typename T, int N>
	PointPair<T, N> box_translate(PointPair<T, N> a, Point<T, N> b)
	{
		PointPair<T, N> c;
		c.p[0] = vec_add(a.p[0], b);
		c.p[1] = vec_add(a.p[1], b);
		return c;
	}

	template <typename T, int N>
	Point<T, N> box_span(PointPair<T, N> a)
	{
		return vec_pair_edge(a);
	}

	template <typename T, int N>
	bool box_empty(PointPair<T, N> a)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (a.p[0].v[i] > a.p[1].v[i])
			{
				return true;
			}
		}

		return false;
	}

	template <typename T, int N>
	Point<T, N> box_center(PointPair<T, N> a)
	{
		return vec_divide<T, N>(vec_add(a.p[0], a.p[1]), 2);
	}

	template <typename T, int N>
	PointPair<T, N> box_surface_facing(PointPair<T, N> a, int facing)
	{
		const int rem = facing & 1;
		const int div = facing >> 1;
		a.p[rem].v[div] = a.p[1 - rem].v[div];
		a.p[1 - rem].v[div] += lwlib_OneMinusNx2(rem);
		return a;
	}

	template <typename T>
	T box3_face_visible(const PointPair<T, 3> box, int facing, 
		Point<T, 3> viewPos)
	{
		Point<T, 3> p;

		p = box3_face_vert(box, facing, 0);

		if (facing < 0)
		{
			facing = -(facing + 1);
		}
		return (viewPos.v[facing >> 1] - p.v[facing >> 1]) * 
			(T)lwlib_OneMinusNx2(facing & 1);
	}

	static inline bool box3i_face_visible(Box3i box, int facing, 
		Point3i viewPos)
	{
		return box3_face_visible(box, facing, viewPos) >= (facing & 1);
	}

	template <typename T, int N>
	char *box_to_string(lwlib_String_t string, const char *fmt,
		PointPair<T, N> a)
	{
		lwlib_String_t lowerStr, upperStr;
		vec_to_string(lowerStr, fmt, a.p[0]);
		vec_to_string(upperStr, fmt, a.p[1]);
		return lwlib_VaString(string, "[ %s %s ]", lowerStr, upperStr);
	}

	template <typename T, int N>
	bool box_equal(PointPair<T, N> a, PointPair<T, N> b, T eps)
	{
		return vec_equal(a.p[0], b.p[0], eps) &&
			vec_equal(a.p[1], b.p[1], eps);
	}

	template <typename T, typename R, int N, int M>
	PointPair<R, M> box_convert(PointPair<T, N> a)
	{
		int i;
		PointPair<R, M> b;

		for (i = 0; i < 2; i++)
		{
			b.p[i] = vec_convert<T, R, N, M>(a.p[i]);
		}

		return b;
	}

	static inline Box3i box3f_int(Box3f a)
	{
		return box_convert<double, int, 3, 3>(a);
	}

	static inline Box3f box3i_float(Box3i a)
	{
		return box_convert<int, double, 3, 3>(a);
	}

	template <typename T, int N>
	bool box_contains(PointPair<T, N> a, PointPair<T, N> b)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (a.p[0].v[i] > b.p[0].v[i] ||
				a.p[1].v[i] < b.p[1].v[i])
			{
				return false;
			}
		}

		return true;
	}

	template <typename T, int N>
	PointPair<T, N> box_normalize(PointPair<T, N> a)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (a.p[0].v[i] > a.p[1].v[i])
			{
				lwlib_Swap(a.p[0].v[i], a.p[1].v[i], T);
			}
		}

		return a;
	}

	template <typename T, int N>
	bool box_test(PointPair<T, N> a, Point<T, N> b)
	{
		int i;

		for (i = 0; i < N; i++)
		{
			if (b.v[i] < a.p[0].v[i] || 
				b.v[i] >= a.p[1].v[i])
			{
				return false;
			}
		}

		return true;
	}

	template <typename T, int N>
	Point<T, N> box_corner(PointPair<T, N> a, int index)
	{
		int i;
		Point<int, N> b;
		Point<T, N> c;

		b = vec_from_bits<int, N>(index);
		for (i = 0; i < N; i++)
		{
			c.v[i] = a.p[b.v[i]].v[i];
		}

		return c;
	}

	static inline double invert_angle(double angle)
	{
		return normalize_angle(-angle);
	}

	static inline int negate_axialplane(int planeNum)
	{
		return (planeNum & ~1) | (1 - (planeNum & 1));
	}

	/* more vector math */

	extern int ray3f_hit_box(Ray3f ray, Box3f box);
	extern int ray3f_hit_plane(Ray3f ray, Plane3f plane, Point3f *hit);

	extern Plane3f box3f_face_plane(const Box3f box, int facing);
	extern Box3f box3f_rotate(Box3f a, Mat3f b);

	extern Sphere3f sphere3f(Point3f origin, double radius);

	/* line-object intersections */

	typedef struct {
		Point3f point;
		Vector3f direction;
	} Line_t;

	typedef struct {
		Point3f vertex;
		Vector3f axis;
		double angle;
		double lowerRim;          /* distance to lower rim or 0 */
		double upperRim;          /* distance to upper rim or 0 */
		int acceptRay;            /* 1: accept ray through cone, 0: reject ray through cone */
		int trivialTest;          /* 0: use general solution, 1: use trivial solution */
		double minRayProj;        /* minimum bound on ray normal projection (trivial solution) */
		double maxRayProj;        /* maximum bound on ray normal projection (trivial solution) */
	} Cone_t;

	extern int hit_line_cone(Line_t line, Cone_t cone, Point3f *hit);

	/* array rotations */

	template <typename T>
	void vecn_rot(T *vec, int n)
	{
		int i;
		T s;

		assert(n > 0);
		s = vec[0];

		for (i = 0; i < n - 1; i++)
		{
			vec[i] = vec[i + 1];
		}

		vec[n - 1] = s;
	}

	template <typename T>
	void vecn_rot_k(T *arr, int n, int k)
	{
		int i;

		while (k < 0)
		{
			k += n;
		}
		k %= n;

		for (i = 0; i < k; i++)
		{
			vecn_rot(arr, n);
		}
	}

	namespace OverlapTest
	{
		enum e
		{
			Touch,
			Containment,
		};
	}

	bool vec3f_box_inside_sphere(Box3f box, Sphere3f sphere);
	bool vec3f_box_touches_sphere(Box3f box, Sphere3f sphere);
	bool vec3f_box_halfspace_overlap(Box3f box, Plane3f plane, 
		OverlapTest::e test);
	bool vec3f_box_sphere_overlap(Box3f box, Sphere3f sphere, 
		OverlapTest::e test);

	/* color space conversions */

	extern Point3f rgb_to_cielab(Point3f rgb);
}

#endif

