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

#ifndef LWLIB_MISC_H
#define LWLIB_MISC_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "fixedptc.h"

typedef unsigned char lwlib_Byte_t;
typedef unsigned long int lwlib_Word_t;
typedef unsigned char lwlib_Bool_t;

#define lwlib_True ((lwlib_Bool_t)(1))
#define lwlib_False ((lwlib_Bool_t)(0))

#define lwlib_Zero(x) memset(&(x), 0, sizeof(x))

#define lwlib_Copy(dst, src) memcpy(&(dst), &(src), sizeof(dst))
#define lwlib_Same(dst, src) (memcmp(&(dst), &(src), sizeof(dst)) == 0)

#define lwlib_ResetElem(ptr, dst, src) \
	if ((ptr) != NULL) \
	{ \
		(ptr) = (dst) + ((ptr) - (src)); \
	}

#define lwlib_CountArray(x) (sizeof(x) / sizeof((x)[0]))
#define lwlib_AssertArrayRange(x, a) assert((x) >= 0 && (size_t)(x) < lwlib_CountArray(a))

#define lwlib_RepeatArrayN(array, count, indexname, elemname, stmts) \
do { \
	int indexname; \
	for (indexname = 0; (size_t)indexname < count; indexname++) \
	{ \
		typeof((array)[0]) elemname = (array)[indexname]; \
		(void)elemname; \
		stmts; \
		(array)[indexname] = elemname; \
	} \
} while(0)

#define lwlib_RepeatArrayNRev(array, count, indexname, elemname, stmts) \
do { \
	int indexname; \
	for (indexname = (count) - 1; indexname >= 0; indexname--) \
	{ \
		typeof((array)[0]) elemname = (array)[indexname]; \
		(void)elemname; \
		stmts; \
		(array)[indexname] = elemname; \
	} \
} while(0)

#define lwlib_RepeatArray(array, indexname, elemname, stmts) \
	lwlib_RepeatArrayN(array, lwlib_CountArray(array), indexname, elemname, stmts)

#define lwlib_RepeatN(count, indexname, stmts) \
do { \
	int indexname; \
	for (indexname = 0; indexname < count; indexname++) \
	{ \
		stmts; \
	} \
} while(0)

// sparse ones algorithm for bit counting
static inline int lwlib_BitCount(unsigned int n)
{
	int count = 0;

	while (n)
	{
		count++;
		n &= (n - 1);
	}

	return count;
}

#define lwlib_IntegerIsPow2(x) (lwlib_BitCount(x) == 1)

#define lwlib_GetPriv(type, obj) \
	type *priv = (type *)(obj).priv; \
	(void)priv

#define lwlib_GetPrivEx(type, obj, privEx) \
	type *privEx = (type *)(obj).priv; \
	(void)privEx

#define lwlib_GetPrivAssert(type, obj) \
	lwlib_GetPriv(type, obj); \
	assert(priv != NULL); \

#define lwlib_GetPrivAssertEx(type, obj, privEx) \
	type *privEx = (type *)(obj).priv; \
	assert(privEx != NULL); \

#define lwlib_StrCopy(dest, src) \
do { \
	strncpy(dest, src, sizeof(dest) - 1); \
	(dest)[sizeof(dest) - 1] = '\0'; \
} while(0)

#define lwlib_Snprintf(dest, fmt, ...) \
do { \
	snprintf(dest, sizeof(dest) - 1, fmt, __VA_ARGS__); \
	dest[sizeof(dest) - 1] = '\0'; \
} while(0)

#define lwlib_BitOn(value, index) (((value) & (1 << (index))) != 0)
#define lwlib_BitOff(value, index) (!lwlib_BitOn(value, index))

#define lwlib_SetBit(value, index, bit) \
	( \
		((value) & ~(1 << (index))) | \
		((bit) ? (1 << (index)) : 0) \
	)

#define lwlib_SetBitLValue(value, index, bit) \
	value = lwlib_SetBit(value, index, bit)

#define lwlib_EnumStrExtern(type) \
	extern const char *type ## _Str[]; \
	extern const int type ## _StrCount

#define lwlib_EnumStr(type, ...) \
	const char *type ## _Str[] = \
	{ \
		__VA_ARGS__ \
	}; \
	const int type ## _StrCount = lwlib_CountArray(type ## _Str)

#define lwlib_EnumValExtern(type) \
	extern const int type ## _Val[]; \
	extern const int type ## _ValCount

#define lwlib_EnumVal(type, ...) \
	const int type ## _Val[] = \
	{ \
		__VA_ARGS__ \
	}; \
	const int type ## _ValCount = lwlib_CountArray(type ## _Val)

#define lwlib_StringSize 128

typedef char lwlib_String_t[lwlib_StringSize];

namespace lwlib
{
	extern char *Va(const char *format, ...);

	extern char *VaString(lwlib_String_t string, const char *format, ...);
}

extern char *lwlib_VaString(lwlib_String_t string, const char *format, ...);

extern char *lwlib_StringToLower(lwlib_String_t string);

extern char *lwlib_StringToUpper(lwlib_String_t string);

#define lwlib_BoolString(b) ((b) ? "true" : "false")

typedef struct lwlib_Str_s
{
	lwlib_String_t v;
} lwlib_Str_t;

static inline lwlib_Str_t lwlib_Str(const char *v)
{
	lwlib_Str_t str;
	lwlib_StrCopy(str.v, v);
	return str;
}

static inline lwlib_Str_t lwlib_Str_FromChar(char ch)
{
	lwlib_Str_t str;
	str.v[0] = ch;
	str.v[1] = '\0';
	return str;
}

static inline lwlib_Str_t lwlib_Str_SubStr(lwlib_Str_t a, int i, int j)
{
	int len;
	lwlib_Str_t str;
	const char *v;

	v = a.v;

	assert
		(
			v != NULL && i >= 0 && (size_t)i <= strlen(v) && 
			(
				j == -1 || 
				(
					j >= 0 && (size_t)j <= strlen(v) && i <= j
				)
			)
		);

	len = (j == -1 ? (strlen(v) - i) : (j - i));
	memcpy(str.v, &v[i], len);
	str.v[len] = '\0';

	return str;
}

static inline lwlib_Str_t lwlib_Str_Cat(lwlib_Str_t a, lwlib_Str_t b)
{
	lwlib_Str_t c;
	lwlib_VaString(c.v, "%s%s", a.v, b.v);
	return c;
}

static inline lwlib_Str_t lwlib_Str_InsertStr(lwlib_Str_t a, lwlib_Str_t b, int i)
{
	assert(i >= 0 && (size_t)i <= strlen(a.v));
	return lwlib_Str_Cat(lwlib_Str_SubStr(a, 0, i), 
		lwlib_Str_Cat(b, lwlib_Str_SubStr(a, i, -1)));
}

static inline lwlib_Str_t lwlib_Str_DeleteChar(lwlib_Str_t a, int i)
{
	assert(i >= 0 && (size_t)i < strlen(a.v));
	return lwlib_Str_Cat(
		lwlib_Str_SubStr(a, 0, i), 
		lwlib_Str_SubStr(a, i + 1, -1)
		);
}

static inline int lwlib_Str_IndexOf(lwlib_Str_t a, char ch)
{
	char *p;
	p = strchr(a.v, ch);
	return p != NULL ? (int)(p - a.v) : -1;
}

static inline lwlib_Str_t lwlib_Str_Cut(lwlib_Str_t a, char ch, int field)
{
	int index;
	lwlib_Str_t b, c;

	c = a;

	while (field >= 0)
	{
		index = lwlib_Str_IndexOf(c, ch);
		if (index == -1)
		{
			return c;
		}
		b = lwlib_Str_SubStr(c, 0, index);
		if (field == 0)
		{
			return b;
		}
		c = lwlib_Str_SubStr(c, index + 1, -1);
		field--;
	}

	return a;
}

lwlib_Str_t lwlib_Str_ToLower(lwlib_Str_t a);

lwlib_Str_t lwlib_Str_ToUpper(lwlib_Str_t a);

#define lwlib_CharLits_a_to_z "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "t", "s", "u", "v", "w", "x", "y", "z"
#define lwlib_CharLits_A_to_Z "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "T", "S", "U", "V", "W", "X", "Y", "Z"
#define lwlib_CharLits_0_to_9 "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"

static inline bool lwlib_IntegerInRange(int a, int b, int c)
{
	return a >= b && a < c;
}

#define lwlib_Pow2(x) (1 << (x))

static inline int lwlib_Log2(int x)
{
	int res = 0;
	assert(x >= 1);
	while (x > 1)
	{
		res++;
		x >>= 1;
	}
	return res;
}

#define lwlib_Swap(a, b, t) \
do { \
	t c = a; \
	a = b; \
	b = c; \
} while (0)

#define lwlib_DegToRad(a) (((a) * M_PI) / 180.0)
#define lwlib_RadToDeg(a) (((a) * 180.0) / M_PI)

#define lwlib_Sq(a) ((a) * (a))
#define lwlib_Sign(a) ((a) > 0.0 ? 1.0 : ((a) < 0.0 ? -1.0 : 0.0))
#define lwlib_SignInt(a) ((a) > 0 ? 1 : ((a) < 0 ? -1 : 0))

#define lwlib_OneMinusNx2(a) (1 - ((a) * 2))

#define lwlib_ObjNullDefn(type) \
static inline type ## _t type ## _Null(void) \
{ \
	type ## _t val; \
	lwlib_Zero(val); \
	return val; \
}

typedef struct
{
	void *priv;
} lwlib_Object_t;

static inline lwlib_Object_t lwlib_Object(void *priv)
{
	lwlib_Object_t ob;
	ob.priv = priv;
	return ob;
}

#define lwlib_CalcKb(x) ((x) * 1024)
#define lwlib_CalcMb(x) (lwlib_CalcKb((x) * 1024))

int lwlib_strncasecmp(const char *s1, const char *s2, int n);

int lwlib_strcasecmp(const char *s1, const char *s2);

typedef struct lwlib_Frac_s
{
    int val;
    int residual;
    int numerator, divider, sign;
} lwlib_Frac_t;

static inline lwlib_Frac_t lwlib_FracInit(int argVal, int argResidual, 
	int argNumerator, int argDivider)
{
	lwlib_Frac_t x;
	x.val = argVal;
	x.residual = argResidual;
	x.numerator = abs(argNumerator);
	x.divider = argDivider;
	x.sign = argNumerator >= 0 ? 1 : -1;
	return x;
}

static inline void lwlib_FracInc(lwlib_Frac_t *x)
{
	x->residual += x->numerator;
	while (x->residual >= x->divider)
	{
		x->val += x->sign;
		x->residual -= x->divider;
	}
}

static inline void lwlib_FracIncMany(lwlib_Frac_t *x, int y)
{
	int n;
	x->residual += x->numerator * y;
	if (x->residual >= x->divider)
	{
		n = x->residual / x->divider;
		x->val += x->sign * n;
		x->residual -= x->divider * n;
	}
}

namespace lwlib
{
	class FixedFrac
	{
	public:
		fixedpt val;
		fixedpt inc;

		FixedFrac() : val(0), inc(0)
		{
		}
	};
}

typedef lwlib::FixedFrac lwlib_FixedFrac_t;

static inline lwlib_FixedFrac_t lwlib_FixedFrac(fixedpt val, fixedpt inc)
{
	lwlib_FixedFrac_t x;
	x.val = val;
	x.inc = inc;
	return x;
}

static inline lwlib_FixedFrac_t lwlib_FixedFracInit(fixedpt argVal,
	fixedpt argNumerator, int argDivider)
{
	lwlib_FixedFrac_t x;
	x.val = argVal;
	x.inc = argNumerator / argDivider;
	return x;
}

static inline lwlib_FixedFrac_t lwlib_FixedFracFromInt(int argVal,
	int argNumerator, int argDivider)
{
	return lwlib_FixedFracInit(fixedpt_fromint(argVal),	
		fixedpt_fromint(argNumerator), argDivider);
}

static inline void lwlib_FixedFracInc(lwlib_FixedFrac_t *x)
{
	x->val += x->inc;
}

static inline void lwlib_FixedFracIncMany(lwlib_FixedFrac_t *x, int y)
{
	x->val += x->inc * y;
}

static inline int lwlib_FixedFracToInt(lwlib_FixedFrac_t *x)
{
	return fixedpt_toint(x->val);
}

namespace lwlib
{
	class FloatFrac
	{
	public:
		float val;
		float inc;

		FloatFrac() : val(0.0), inc(0.0)
		{
		}
	};
}

typedef lwlib::FloatFrac lwlib_FloatFrac_t;

static inline lwlib_FloatFrac_t lwlib_FloatFracInit(float argVal,
	float argNumerator, int argDivider)
{
	lwlib_FloatFrac_t x;
	x.val = argVal;
	x.inc = argNumerator / argDivider;
	return x;
}

static inline void lwlib_FloatFracInc(lwlib_FloatFrac_t *x)
{
	x->val += x->inc;
}

static inline void lwlib_FloatFracIncMany(lwlib_FloatFrac_t *x, int y)
{
	x->val += x->inc * y;
}

static inline int lwlib_FloatFracToInt(lwlib_FloatFrac_t *x)
{
	return (int)(x->val);
}

#define lwlib_ObjValid(obj) ((obj) != NULL)

int lwlib_IndexOf(int val, const int *values, int numValues);

#define lwlib_AnonCallback_t(func) struct { void *anon; func; }

#define lwlib_AnonCallbackInit(anonCallback, anonObj, func) \
do { \
	(anonCallback).anon = (void *)anonObj; \
	(anonCallback).fn = func; \
} while (0)

#define lwlib_DefineDelegate(name, ret, ...) \
typedef struct \
{ \
	void *obj; \
	ret (*fn)(__VA_ARGS__); \
} name

#define lwlib_InitDelegate(name, ob, f) \
do { \
	delegateAPI->name.obj = ob; \
	delegateAPI->name.fn = f; \
} while(0)

#define lwlib_DeclDelegateAPI(type, obj) \
	type *delegateAPI = (type *)&(obj)->priv.delegateAPI

#define lwlib_CallDelegateNoArgs(delegate) \
	delegateAPI->delegate.fn(delegateAPI->delegate.obj)

#define lwlib_CallDelegate(delegate, ...) \
	delegateAPI->delegate.fn(delegateAPI->delegate.obj, __VA_ARGS__)

#define lwlib_RandInt(range) ((int)(((uint64_t)(range) * \
	(uint64_t)rand()) / (uint64_t)RAND_MAX))

#endif
