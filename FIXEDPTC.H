#ifndef _FIXEDPTC_H_
#define _FIXEDPTC_H_

/*
 * Signed 32-bit fixed point number library for the 24.8 format.
 * The specific limits are -8388608.999... to 8388607.999... and the
 * most precise number is 0.00390625. In practice, you should not count
 * on working with numbers larger than a million or to the precision
 * of more than 2 decimal places. Make peace with the fact that PI
 * is 3.14 here. :)
 *
 * Since the precision is relatively low, many complex functions (more
 * complex than div & mul) take a large hit on the precision of the
 * end result, because errors in precision accumulate.
 * This loss of precision can be lessened by increasing the number of
 * bits dedicated to the fraction part, but at the loss of range.
 *
 * Adventurous users might utilize this library to build two data types:
 * one which has the range, and one which has the precision, and carefully
 * convert between them (including adding two number of each type to produce
 * a simulated type with a larger range and precision).
 *
 * The ideas and algorithms have been cherry-picked from a large number
 * of previous implementations available on the Internet.
 */

/*-
 * Copyright (c) 2010 Ivan Voras <ivoras@freebsd.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32)
#	include <stdint.h>
#	include <string.h>
#	include <stdarg.h>
#endif
#include <stdint.h>

typedef int32_t fixedpt;

/* Actually, you can redefine the FIXEDPT_WBITS constant to support other
 * divisions of the 32-bit integer, but who wants to work with 16-bit integers
 * these days? :)
 *
 * However, extending the base type to 64-bit would require approximately the
 * same work as rewriting this library from scratch: such implementation would
 * have to deal manually with overflows (there is no 128-bit data type in C),
 * and the magic numbers (there are a lot of them!) would have to be
 * recalculated for 64-bit operations.
 */

#define FIXEDPT_BITS	32
#ifndef FIXEDPT_WBITS
#define FIXEDPT_WBITS	16 // 24
#endif
#define FIXEDPT_FBITS	(FIXEDPT_BITS - FIXEDPT_WBITS)
#define FIXEDPT_FMASK	((1 << FIXEDPT_FBITS) - 1)

#define fixedpt_rconst(R) (int32_t)((R) * (1LL << FIXEDPT_FBITS) + ((R) >= 0 ? 0.5 : -0.5))
#define fixedpt_fromint(I) ((int64_t)(I) << FIXEDPT_FBITS)
#define fixedpt_toint(F) ((F) >> FIXEDPT_FBITS)
#define fixedpt_toconst(F) ((double)(F) / (1LL << FIXEDPT_FBITS))
#define fixedpt_add(A,B) ((A) + (B))
#define fixedpt_sub(A,B) ((A) - (B))
#define fixedpt_xmul(A,B) (int32_t)(((int64_t)(A) * (int64_t)(B)) >> FIXEDPT_FBITS)
#define fixedpt_xdiv(A,B) (int32_t)(((int64_t)(A) << FIXEDPT_FBITS) / (int64_t)(B))
#define fixedpt_fracpart(A) ((A) & FIXEDPT_FMASK)

#define FIXEDPT_ONE	(int32_t)(1 << FIXEDPT_FBITS)
#define FIXEDPT_ONE_HALF (FIXEDPT_ONE >> 1)
#define FIXEDPT_TWO	(FIXEDPT_ONE + FIXEDPT_ONE)
#define FIXEDPT_PI	fixedpt_rconst(3.14159265)
#define FIXEDPT_TWO_PI	fixedpt_rconst(2*3.14159265)
#define FIXEDPT_HALF_PI	fixedpt_rconst(3.14159265/2)
#define FIXEDPT_E	fixedpt_rconst(2.71828183)

#define fixedpt_abs(A) ((A) < 0 ? -(A) : (A))

/* Multiplies two fixedpt numbers, returns the result. */
static inline int32_t
fixedpt_mul(fixedpt A, fixedpt B)
{
	return (((int64_t)A * (int64_t)B) >> FIXEDPT_FBITS);
}


/* Divides two fixedpt numbers, returns the result. */
static inline int32_t
fixedpt_div(fixedpt A, fixedpt B)
{
	return (((int64_t)A << FIXEDPT_FBITS) / (int64_t)B);
}

/*
 * Note: adding and substracting fixedpt numbers can be done by using
 * the regular integer operators + and -.
 */

/* Convert the given fixedpt number to a decimal string */
static inline void
fixedpt_str(fixedpt A, char *str)
{
	int ndec = 0, slen = 0;
	char tmp[12] = {0};
	uint64_t fr, ip;
	const uint64_t one = 1LL << FIXEDPT_BITS;
	const uint64_t mask = one - 1;

	if (A < 0) {
		str[slen++] = '-';
		A *= -1;
	}

	ip = fixedpt_toint(A);
	do {
		tmp[ndec++] = '0' + ip % 10;
		ip /= 10;
	} while (ip != 0);

	while (ndec > 0)
		str[slen++] = tmp[--ndec];
	str[slen++] = '.';

	fr = (fixedpt_fracpart(A) << FIXEDPT_WBITS) & mask;
	do {
		fr = (fr & mask) * 10;

		str[slen++] = '0' + (fr >> FIXEDPT_BITS) % 10;
		ndec++;
	} while (fr != 0);

	if (ndec > 1 && str[slen-1] == '0')
		str[slen-1] = '\0'; /* cut off trailing 0 */
	else
		str[slen] = '\0';
}


/* Converts the given fixedpt number into a string, using a static
 * (non-threadsafe) string buffer */
static inline char*
fixedpt_cstr(fixedpt A)
{
	static char str[20];

	fixedpt_str(A, str);
	return (str);
}


/* Returns the square root of the given number, or -1 in case of error */
static inline fixedpt
fixedpt_sqrt(fixedpt A)
{
	int invert = 0;
	int iter = FIXEDPT_FBITS;
	int l, i;

	if (A < 0)
		return (-1);
	if (A == 0 || A == FIXEDPT_ONE)
		return (A);
	if (A < FIXEDPT_ONE && A > 6) {
		invert = 1;
		A = fixedpt_div(FIXEDPT_ONE, A);
	}
	if (A > FIXEDPT_ONE) {
		int s = A;

		iter = 0;
		while (s > 0) {
			s >>= 2;
			iter++;
		}
	}

	/* Newton's iterations */
	l = (A >> 1) + 1;
	for (i = 0; i < iter; i++)
		l = (l + fixedpt_div(A, l)) >> 1;
	if (invert)
		return (fixedpt_div(FIXEDPT_ONE, l));
	return (l);
}


/* Returns the sine of the given fixedpt number. 
 * Note: the loss of precision is extraordinary! */
static inline fixedpt
fixedpt_sin(fixedpt fp)
{
	int sign = 1;
	int32_t sqr, result;
	const int sk[2] = { 16342350, 356589659 };
	static int SK[2] = { 0, 0 };

	if (SK[0] == 0) {
		int i;
		for (i = 0; i < 2; i++)
			SK[i] = sk[i] >> (31 - FIXEDPT_FBITS);
	}

	fp %= 2 * FIXEDPT_PI;
	if (fp < 0)
		fp = FIXEDPT_PI * 2 + fp;
	if ((fp > FIXEDPT_HALF_PI) && (fp <= FIXEDPT_PI)) 
		fp = FIXEDPT_PI - fp;
	else if ((fp > FIXEDPT_PI) && (fp <= (FIXEDPT_PI + FIXEDPT_HALF_PI))) {
		fp = fp - FIXEDPT_PI;
		sign = -1;
	} else if (fp > (FIXEDPT_PI + FIXEDPT_HALF_PI)) {
		fp = (FIXEDPT_PI << 1) - fp;
		sign = -1;
	}
	sqr = fixedpt_mul(fp, fp);
	result = SK[0];
	result = fixedpt_mul(result, sqr);
	result -= SK[1];
	result = fixedpt_mul(result, sqr);
	result += FIXEDPT_ONE;
	result = fixedpt_mul(result, fp);
	return sign * result;
}


/* Returns the cosine of the given fixedpt number */
static inline fixedpt
fixedpt_cos(fixedpt A)
{
	return (fixedpt_sin(FIXEDPT_HALF_PI - A));
}


/* Returns the tangens of the given fixedpt number */
static inline fixedpt
fixedpt_tan(fixedpt A)
{
	return fixedpt_div(fixedpt_sin(A), fixedpt_cos(A));
}


/* Returns the value exp(x), i.e. e^x of the given fixedpt number. */
static inline fixedpt
fixedpt_exp(fixedpt fp)
{
	int32_t xabs, k, z, R, xp;
	const int ln2 = 744261117; //0.69314718055994530941723212145818 * 2^30
	const int ln2_inv = 1549082004; //1.4426950408889634073599246810019
	const int exp_p[5] = { 357913941, -5965232, 142029, -3550, 88 };
	static int LN2 = 0;
	static int LN2_INV = 0;
	static int EXP_P[5] = { 0, 0, 0, 0, 0 };

	if (LN2 == 0)
		LN2 = ln2 >> (30 - FIXEDPT_FBITS);
	if (LN2_INV == 0)
		LN2_INV = ln2_inv >> (30 - FIXEDPT_FBITS);
	if (EXP_P[0] == 0) {
		int i;
		for (i = 0; i < 5; i++)
			EXP_P[i] = exp_p[i] >> (31 - FIXEDPT_FBITS);
	}

	if (fp == 0)
		return (FIXEDPT_ONE);
	xabs = fixedpt_abs(fp);
	k = fixedpt_mul(xabs, LN2_INV);
	k += FIXEDPT_ONE_HALF;
	k &= ~FIXEDPT_FMASK;
	if (fp < 0)
		k = -k;
	fp -= fixedpt_mul(k, LN2);
	z = fixedpt_mul(fp, fp);
	/* Taylor */
	R = FIXEDPT_TWO +
	    fixedpt_mul(z, EXP_P[0] + fixedpt_mul(z, EXP_P[1] +
	    fixedpt_mul(z, EXP_P[2] + fixedpt_mul(z, EXP_P[3] +
	    fixedpt_mul(z, EXP_P[4])))));
	xp = FIXEDPT_ONE + fixedpt_div(fixedpt_mul(fp, FIXEDPT_TWO), R - fp);
	if (k < 0)
		k = FIXEDPT_ONE >> (-k >> FIXEDPT_FBITS);
	else
		k = FIXEDPT_ONE << (k >> FIXEDPT_FBITS);
	return (fixedpt_mul(k, xp));
}


/* Returns the natural logarithm of the given fixedpt number. */
static inline fixedpt
fixedpt_ln(fixedpt x)
{
	fixedpt log2, xi;
	fixedpt f, s, z, w, R;
	const int ln2 = 744261117; //0.69314718055994530941723212145818 * 2^30
	const int lg[7] = { 1431655765,	858993459, 613566760, 477218077, 390489238, 328862160, 317788895 };
	static int LN2 = 0;
	static int LG[7] = { 0, 0, 0, 0, 0, 0, 0 };

	if (LN2 == 0)
		LN2 = ln2 >> (30 - FIXEDPT_FBITS);
	if (LG[0] == 0) {
		int i;
		for (i = 0; i < 7; i++)
			LG[i] = lg[i] >> (31 - FIXEDPT_FBITS);
	}

	if (x < 0)
		return (0);
	if (x == 0)
		return 0xffffffff;

	log2 = 0;
	xi = x;
	while (xi > FIXEDPT_TWO) {
		xi >>= 1;
		log2++;
	}
	f = xi - FIXEDPT_ONE;
	s = fixedpt_div(f, FIXEDPT_TWO + f);
	z = fixedpt_mul(s, s);
	w = fixedpt_mul(z, z);
	R = fixedpt_mul(w, LG[1] + fixedpt_mul(w, LG[3] + fixedpt_mul(w, LG[5])))
	    + fixedpt_mul(z, LG[0] + fixedpt_mul(w, LG[2] + fixedpt_mul(w, LG[4] +
	    fixedpt_mul(w, LG[6]))));
	return (fixedpt_mul(LN2, (log2 << FIXEDPT_FBITS)) + f - fixedpt_mul(s, f - R));
}
	

/* Returns the logarithm of the given base of the given fixedpt number */
static inline fixedpt
fixedpt_log(fixedpt x, fixedpt base)
{
	return (fixedpt_div(fixedpt_ln(x), fixedpt_ln(base)));
}


/* Return the power value (n^exp) of the given fixedpt numbers */
static inline fixedpt
fixedpt_pow(fixedpt n, fixedpt exp)
{
	if (exp == 0)
		return (FIXEDPT_ONE);
	if (n < 0)
		return 0;
	return (fixedpt_exp(fixedpt_mul(fixedpt_ln(n), exp)));
}

#endif
