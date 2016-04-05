/*

REALS.cc -- main part of the implementation of reals for the iRRAM library
 
Copyright (C) 2001-2010 Norbert Mueller
 
This file is part of the iRRAM Library.
 
The iRRAM Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
 
The iRRAM Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.
 
You should have received a copy of the GNU Library General Public License
along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. 
*/

#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/REAL.h>
#include <iRRAM/sizetype.hh>
#include <iRRAM/cache.h>
#include <iRRAM/SWITCHES.h>
#include <iRRAM/DYADIC.h>
#include <iRRAM/INTEGER.h>
#include <iRRAM/RATIONAL.h>

#if iRRAM_BACKEND_MPFR
# include "MPFR/MPFR_ext.h"
#else
# error "Currently no further backends defined!"
#endif

/*!
 * \todo
 * REALs with MP-backend are infectious: every operation involving other
 * `double_pair` REALs will result in a MP-backed result even on
 * precision-level 1. Check which of the following functions could be fitted to
 * work on the `double_pair` representation directly or could at least be
 * adapted to return a precision-level 1 result:
 * - sqrt(const REAL &)
 * - power(const REAL &, const REAL &)
 * - sqrt(const REAL &)
 * - approx(const REAL &, const int)
 * - bound(const REAL &, const int)
 * - size(const REAL &)
 * - upperbound(const REAL &)
 * - modulo(const REAL &, const REAL &)
 * - limit() family
 * - REAL::geterror()
 * - REAL::adderror()
 * - REAL::getsize()
 * - REAL::as_INTEGER()
 * - swrite(const REAL &, const int, const float_form)
 * - strtoREAL2(const char *, char **)
 * - pi()
 * - euler()
 * - ln2()
 * - root(const REAL &, int n) at least for `n == 3`
 * - trigonometric functions
 * \todo
 * Special care has to be taken in case C/C++ library functions are used to
 * implement those. Only `sqrt(double)` and `fma(double,double,double)` provide
 * error guarrantees according to the respective standards (for `fma` including
 * POSIX). Standard compliance in this respect also cannot be assumed for every
 * C/C++ standard library but should probably be white-listed for proven
 * implementations. See
 * - sizetype_sqrt()
 */

/*!
 * \todo
 * The functions REAL::geterror(), REAL::getsize() (and maybe REAL::adderror()?)
 * are sometimes used to implicitely convert to an MP-backed REAL. That's not
 * part of their contract. Identify and fix those.
 */

namespace iRRAM {

void REAL::mp_make_mp()
{
	if (!std::isfinite(dp.upper_neg) || !std::isfinite(dp.lower_pos))
		iRRAM_REITERATE(0);
	if (dp.upper_neg == -dp.lower_pos) {
		// here we have a point interval...
		if (!value)
			MP_init(value);
		MP_double_to_mp(dp.lower_pos, value);
		error = sizetype_exact();
	} else {
		// now we know that it is not a point interval:
		MP_init(value);
		MP_double_to_mp(dp.lower_pos, value);
		MP_type value1;
		MP_init(value1);
		MP_double_to_mp(-dp.upper_neg, value1);
		MP_type value2;
		MP_init(value2);
		MP_add(value, value1, value2, -1150);
		MP_shift(value2, value, -1);
		MP_clear(value1);
		MP_clear(value2);
		// value0 differs from precise result (lower+upper)/2 by at most
		// 2^{-1200},
		// which is sufficiently more precise than all bits even of the
		// smallest double number (2^-1075), so here the error is as
		// small as reasonable
		double rwidth = (dp.upper_neg + dp.lower_pos);
		int e;
		unsigned m = (unsigned)ldexp(frexp(-rwidth, &e), 30) + 2;
		// Here, the "+2" accounts for the possible truncation error and
		// the error on the computation of cvalue.
		/* round to -\infty => dp.upper-dp.lower <= -rwidth <= m*2^(e-30) */
		// So we have that the interval (dp.lower,dp.upper) is a subset
		// of the interval (cvalue - m*2^(e-29),cvalue + m*2^(e-29))
		error = sizetype_normalize({m, e - 29});
	}
	MP_getsize(value, vsize);
}

void REAL::mp_from_mp(const REAL & y)
{
	// d=MP_mp_to_double(y) is required to return a value d such that
	// there is no other double between d and x.
	// the conversion should also generate infinities in case of overflow
	// we don't require a directed rounding here!
	double center = MP_mp_to_double(y.value);
	// in consequence, wd will be the negative of an upper bound
	// for the width of the interval!
	double wd = ldexp(-double(y.error.mantissa), y.error.exponent);
	dp.upper_neg = nextafter(-center, -INFINITY) + wd;
	dp.lower_pos = nextafter(center, -INFINITY) + wd;
}

void REAL::mp_from_int(const int i)
{
	MP_init(value);
	MP_int_to_mp(i, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

void REAL::mp_from_double(const double d)
{
	MP_init(value);
	MP_double_to_mp(d, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

void REAL::mp_copy(const REAL & y)
{
	MP_duplicate_wo_init(y.value, value);
	error = y.error;
	vsize = y.vsize;
}

void REAL::mp_copy_init(const REAL & y)
{
	MP_duplicate_w_init(y.value, value);
	error = y.error;
	vsize = y.vsize;
}

REAL REAL::mp_addition(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  stack.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + stack.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_add(this->value, y.value, zvalue, local_prec);

	sizetype_add_wo_norm(zerror, this->error, y.error);
	zerror = sizetype_add_power2(zerror, local_prec);

	return REAL(zvalue, zerror);
}

REAL REAL::mp_addition(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec = 0;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(this->error.exponent, stack.actual_prec);
	else {
		sizetype ysize;
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(this->vsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + stack.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_addi(this->value, n, zvalue, local_prec);
	zerror = sizetype_add_power2(this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL & REAL::mp_eqaddition(const REAL & y)
{
	MP_type zvalue;
	int local_prec;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  stack.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + stack.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_add(this->value, y.value, zvalue, local_prec);

	this->error += y.error;
	this->error = sizetype_add_power2(this->error, local_prec);

	/*  zerror = sizetype_power2(local_prec);
	  sizetype_inc2(this->error,y.error,zerror);*/

	MP_clear(this->value);
	this->value = zvalue;
	MP_getsize(this->value, this->vsize);
	return (*this);
}

std::string swrite(const REAL & x, const int w, const float_form form)
{
	if (!x.value) {
		REAL y(x);
		y.mp_make_mp();
		return swrite(y, w, form);
	}

	std::string result;
	if (get_cached(result))
		return result;

	int width = w;
	if (width < 9)
		width = 9;
	char * erg;

	switch (form) {
	case float_form::absolute: {
		sizetype psize;
		int p = -10 * (width - 8) / 3;
		psize = sizetype_power2(p);
		int s = MP_size(x.value);
		int mantissa =
		        (int)((s - x.error.exponent - GUARD_BITS) * .30103);
		if (sizetype_less(psize, x.error) ||
		    (s > p && mantissa + 8 < width)) {
			iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
			                "conversion with precision 2^(%d)\n",
			             x.error.mantissa, x.error.exponent, p);
			iRRAM_REITERATE(p - x.error.exponent);
		}

		if (s <= p) {
			erg = (char *)calloc(width + 1, sizeof(char));
			strcpy(erg, "  0 ");
			for (int j = 4; j < width; j++)
				erg[j] = ' ';
			erg[width] = 0;
		} else {
			erg = MP_swrite(x.value, width);
		}
		break;
	}
	case float_form::relative: {
		int p;
		{
			single_valued code;
			// We want to have an error less than 0.65 ulp, where
			// 0.51 ulp come from
			// the conversion to decimal, so we need to know that
			// the error is at most
			// 0.15ulp.
			// At the time, we take the size of x....
			// It would be better to take the x.size, subtract
			// x.error and approximately
			// convert the result to decimal in order to get a
			// correct approximation
			// for a lower bound of x. Using this lower bound we can
			// estimate what
			// 0.15ulp means in binary.
			p = size(x) - 2 - 10 * (width - 8) / 3;
			// We have to prevent that "size" changes the
			// multi-value-cache here!
			// Otherwise, any iteration would not use the same
			// amount of
			// cache as the first run!
		}
		sizetype psize;
		psize = sizetype_power2(p);
		int s = MP_size(x.value);
		int mantissa =
		        (int)((s - x.error.exponent - GUARD_BITS) * .30103);
		if (sizetype_less(psize, x.error) ||
		    (s > p && mantissa + 8 < width)) {
			iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
			                "writing with precision 2^(%d)\n",
			             x.error.mantissa, x.error.exponent, p);
			iRRAM_REITERATE(p - x.error.exponent);
		}
		erg = MP_swrite(x.value, width);
		break;
	}
	case float_form::show: {
		int mantissa = (int)(int((MP_size(x.value) - x.error.exponent -
		                          GUARD_BITS)) *
		                     .30103);
		if (mantissa < 1) {
			erg = (char *)calloc(width + 1, sizeof(char));
			sprintf(erg, " .*E%05.0f",
			        1 + (x.error.exponent + GUARD_BITS) * .30103);
			for (int j = 9; j < width; j++)
				erg[j] = ' ';
		} else {
			if (mantissa > width - 8)
				mantissa = width - 8;
			char * erg2 = MP_swrite(x.value, mantissa + 8);
			erg = (char *)calloc(width + 1, sizeof(char));
			strncpy(erg, erg2, mantissa + 8);
			for (int j = mantissa + 8; j < width; j++)
				erg[j] = ' ';
			erg[width] = 0;
			free(erg2);
		}
		break;
	}
	}

	result = erg;
	free(erg);
	put_cached(result);
	return result;
}

REAL REAL::mp_subtraction(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max({y.error.exponent, this->error.exponent,
		                  stack.actual_prec});
	else {
		local_prec = max(this->vsize.exponent, y.vsize.exponent);
		local_prec = max({y.error.exponent, this->error.exponent,
		                  local_prec - 50 + stack.actual_prec});
	}
	MP_init(zvalue);
	MP_mv_sub(this->value, y.value, zvalue, local_prec);

	sizetype_add_wo_norm(zerror, this->error, y.error);
	zerror = sizetype_add_power2(zerror, local_prec);
	/*
	  zerror = sizetype_power2(local_prec);
	  sizetype_inc2(zerror,this->error,y.error);
	*/
	return REAL(zvalue, zerror);
}

REAL REAL::mp_subtraction(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec =
		        max(this->error.exponent, stack.actual_prec);
	else {
		sizetype ysize;
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(this->vsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + stack.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_subi(this->value, n, zvalue, local_prec);
	zerror = sizetype_add_power2(this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_invsubtraction(const int n) const
{
	MP_type zvalue;
	sizetype zerror;
	int local_prec;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(this->error.exponent, stack.actual_prec);
	else {
		sizetype xsize, ysize;
		MP_getsize(this->value, xsize);
		ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
		local_prec = max(xsize.exponent, ysize.exponent);
		local_prec = max(this->error.exponent,
		                 local_prec - 50 + stack.actual_prec);
	}
	MP_init(zvalue);
	MP_mv_isub(n, this->value, zvalue, local_prec);
	zerror = sizetype_add_power2(this->error, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_multiplication(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror, proderror, sumerror;
	int local_prec;
	zerror = this->vsize * y.error;
	sizetype_add_wo_norm(sumerror, y.vsize, y.error);
	proderror = sumerror * this->error;
	zerror += proderror;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(zerror.exponent, stack.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + y.vsize.exponent - 50 +
		                         stack.actual_prec);
	MP_init(zvalue);
	MP_mv_mul(this->value, y.value, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_multiplication(const int n) const
{
	MP_type zvalue;
	sizetype zerror, ysize;
	int local_prec;
	ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
	zerror = ysize * this->error;

	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(zerror.exponent, stack.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + ysize.exponent - 50 +
		                         stack.actual_prec);
	MP_init(zvalue);
	MP_mv_muli(this->value, n, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_division(const REAL & y) const
{
	MP_type zvalue;
	sizetype zerror, h1, h2, h3;
	int local_prec;
	sizetype_half(h1, y.vsize);
	if (sizetype_less(h1, y.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in "
		                "denominator of size %d*2^(%d)\n",
		             y.error.mantissa, y.error.exponent,
		             y.vsize.mantissa, y.vsize.exponent);
		iRRAM_REITERATE(0);
	}
	h1 = this->vsize * y.error;
	h2 = y.vsize * this->error;
	h1 += h2;
	h3 = y.vsize;
	sizetype_dec(h3);
	sizetype_dec(h3, y.error);
	h2 = h3 * y.vsize;
	sizetype_div(zerror, h1, h2);
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(zerror.exponent, stack.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent - y.vsize.exponent - 50 +
		                         stack.actual_prec);
	MP_init(zvalue);
	MP_mv_div(this->value, y.value, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	return REAL(zvalue, zerror);
}

REAL REAL::mp_division(const int n) const
{
	MP_type zvalue;
	sizetype zerror, ysize;
	int local_prec;
	ysize = sizetype_normalize({(unsigned)(n > 0 ? n : -n), 0});
	sizetype_div(zerror, this->error, ysize);
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(zerror.exponent, stack.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent - ysize.exponent - 50 +
		                         stack.actual_prec);
	MP_init(zvalue);
	MP_mv_divi(this->value, n, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	return REAL(zvalue, zerror);
}


void rwrite(const REAL & x, const int w)  { cout << swrite(x, w, float_form::absolute); }
void rwritee(const REAL & x, const int w) { cout << swrite(x, w, float_form::relative); }
void rshow(const REAL & x, const int w)   { cout << swrite(x, w, float_form::show); }


REAL REAL::mp_square() const
{
	MP_type zvalue;
	sizetype zerror, proderror;
	int local_prec;
	zerror = this->vsize * this->error;
	proderror = this->vsize * this->error;
	zerror += proderror;
	proderror = this->error * this->error;
	zerror += proderror;
	const auto &stack = actual_stack();
	if (stack.prec_policy == 0)
		local_prec = max(zerror.exponent, stack.actual_prec);
	else
		local_prec = max(zerror.exponent,
		                 this->vsize.exponent + this->vsize.exponent -
		                         50 + stack.actual_prec);
	MP_init(zvalue);
	MP_mv_mul(this->value, this->value, zvalue, local_prec);
	zerror = sizetype_add_power2(zerror, local_prec);
	return REAL(zvalue, zerror);
}

LAZY_BOOLEAN REAL::mp_less(const REAL & y) const
{
	REAL z = y - (*this);
	sizetype s;
	MP_getsize(z.value, s);
	if (sizetype_less(s, z.error)) {
		iRRAM_DEBUG2(1, "insufficient precisions %d*2^(%d) and "
		                "%d*2^(%d) in comparing\n",
		             this->error.mantissa, this->error.exponent,
		             y.error.mantissa, y.error.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	return ((MP_sign((z.value)) == 1));
}

REAL REAL::mp_absval() const
{
	MP_type zvalue;
	MP_init(zvalue);
	MP_abs(value, zvalue);
	return REAL(zvalue, error);
}

// REAL REAL::mp_interval_join (const REAL& y)const
// {
// /* The purpose of this routine is to compute interval hull (as a simplified
//  * interval) to the union of two intervals (where both are also given a
//  * simplified intervals). */
// 
// error("this is unfinished!")
// 
//   MP_type zvalue; sizetype zerror; int local_prec;
//   if (ACTUAL_STACK.prec_policy == 0)
//   local_prec=max({y.error.exponent,this->error.exponent,ACTUAL_STACK.actual_prec});
//   else {
//   local_prec=max(this->vsize.exponent,y.vsize.exponent);
//   local_prec=max({y.error.exponent,this->error.exponent,local_prec-50+ACTUAL_STACK.actual_prec});
//   }
//   MP_init(zvalue);
//   MP_mv_add(this->value,y.value,zvalue,local_prec);
// 
//   sizetype_add_wo_norm(zerror,this->error,y.error);
//   zerror = sizetype_add_power2(zerror,local_prec);
// 
//   return REAL(zvalue,zerror);
// }


/*****************************************************/

void REAL::mp_scale(int n)
{
	MP_shift(value, value, n);
	MP_getsize(value, vsize);
	error = error << n;
}

LAZY_BOOLEAN positive(const REAL & x, int k)
{
	if (!x.value) {
		REAL y(x);
		return positive(y.mp_conv(), k);
	}
	bool erg;
	sizetype ksize;
	ksize = sizetype_power2(k);
	if (sizetype_less(ksize, x.error) && sizetype_less(x.vsize, x.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in test on "
		                "positive\n",
		             x.error.mantissa, x.error.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	erg = (MP_sign(x.value) == 1);
	return erg;
}

/*!
 * \brief Computes a dyadic approximation to \a x that is accurate to 2^p.
 *
 * This is a multi-valued function.
 *
 * Reiteration is used to ensure \f$x_\varepsilon\leq2^{p+1}\f$. The computed
 * value at least as precise as \f$x_c\f$ rounded to a multiple of
 * \f$2^{p-1}\f$.
 *
 * \param x the real number to approximate
 * \param p accuracy
 * \return a dyadic approximation q that satisfies |x-q|<2^p.
 * \exception Iteration when \f$2^{p+1}<x_\varepsilon\f$
 * \sa iRRAM_REITERATE
 */
DYADIC approx(const REAL & x, const int p)
{
	if (!x.value)
		return approx(REAL(x).mp_conv(), p);
	MP_type result;
	MP_type erg;
	if (get_cached(result)) {
		MP_duplicate_w_init(result, erg);
		return DYADIC(erg);
	}

	if (sizetype_less(sizetype_power2(p + 1), x.error)) {
		iRRAM_DEBUG2(1,
		             "insufficient precision %d*2^(%d) in approx(%d)\n",
		             x.error.mantissa, x.error.exponent, p);
		iRRAM_REITERATE(p - x.error.exponent);
	}
	MP_init(erg);
	MP_copy(x.value, erg, p - 1);

	if (actual_stack().inlimit == 0) { /* TODO: make state_t::put_cached lambda-aware */
		MP_duplicate_w_init(erg, result);
		put_cached(result);
	}
	return DYADIC(erg);
}

/*!
 * \brief Returns a tight bound to the logarithmic value of |\a x|:
 *        \f$2^{k-2}-2^e\leq|x|<2^k\f$, where \a e is `x.vsize.exponent`.
 *
 * This is a multi-valued function.
 *
 * For \f$\hat x=m\cdot2^e\f$ = `x.vsize` and \f$x_\varepsilon\f$ = `x.error`,
 * this function computes
 * \f$k=\lceil\log_2(\hat x+x_\varepsilon)\rceil>\log_2|x|\f$
 * and via reiteration enforces that
 * \f$\hat x\geq x_\varepsilon+2^{k-2}\f$ which is equivalent to
 * \f$2^{k-2}\leq\hat x-x_\varepsilon=(m-1)\cdot2^e-x_\varepsilon+2^e
 *           \leq|x|+2^e\f$,
 * the guarrantee therefore is \f$2^{k-2}-2^e\leq|x|<2^k\f$.
 *
 * \param x non-zero real number
 * \return \f$k\in\mathbb Z:2^{k-2}-2^e\leq|x|<2^k\f$,
 *         where \a e is `x.vsize.exponent` and
 *         \ref min_exponent <= \a k < \ref MP_max
 * \exception iRRAM_Numerical_Exception(iRRAM_underflow_error)
 *    if x is exact with value zero. If x is not exact but zero, reiterations
 *    will be performed.
 * \sa iRRAM_REITERATE
 * \todo According to documentation, size(const REAL &) should compute
 * \f$\begin{cases}
 *    1+\lfloor\log_2|x|\rfloor\text,&x\neq 0
 * \\ 1+\lceil\log_2|x|\rceil\text,&x\neq 0
 * \\ \text{undef,}&x=0
 * \end{cases}\f$ but it does compute \f$\begin{cases}
 *    i+\lfloor\log_2|x|\rfloor\text,&x\neq 0, i\in\{1,2,3\}
 * \\ \text{undef,}&x=0
 * \end{cases}\f$
 */
int size(const REAL & x)
{
	if (!x.value)
		return size(REAL(x).mp_conv());
	int result = 0;
	if (get_cached(result))
		return result;

	sizetype x_max = x.vsize + x.error;

	if (x_max.mantissa == 0)
		throw iRRAM_Numerical_Exception(iRRAM_underflow_error);

	result = sizetype_log2(x_max);

	if (sizetype_less(x.vsize, sizetype_add_power2(x.error, result - 2))) {
		iRRAM_DEBUG2(
		        1,
		        "insufficient precision %d*2^(%d) in size %d*2^(%d)\n",
		        x.error.mantissa, x.error.exponent, x.vsize.mantissa,
		        x.vsize.exponent);
		iRRAM_REITERATE(0);
	}

	put_cached(result);
	return result;
}

int upperbound(const REAL & x)
{
	if (!x.value) {
		REAL y(x);
		return upperbound(y.mp_conv());
	}
	int result;
	sizetype ergsize;
	if (get_cached(result))
		return result;
	sizetype_add(ergsize, x.vsize, x.error);
	while (ergsize.mantissa > (1 << 16)) {
		ergsize.mantissa >>= 16;
		ergsize.exponent += 16;
	}
	while (ergsize.mantissa > (1 << 4)) {
		ergsize.mantissa >>= 4;
		ergsize.exponent += 4;
	}
	while (ergsize.mantissa > 1) {
		ergsize.mantissa >>= 1;
		ergsize.exponent += 1;
	}
	result = ergsize.exponent;
	put_cached(result);
	return result;
}

/*!
 * \brief Multi-valued check whether 2^k bounds |x|.
 *
 * If it is necessary to check that a number is small enough (e.g. given as the
 * error of an approximating algorithm), bound(x,k) should
 * be preferred over size(x)<k as there is no singularity at x=0.
 *
 * The current approximation to \f$x\f$ is deemed precise enough,
 * predicate \f$P_k(\hat x,x_\varepsilon)\f$, when
 * \f$\neg(\hat x<x_\varepsilon+2^{k-1}\wedge2^k<\hat x+x_\varepsilon)\f$.
 * Otherwise, \f$\bot\f$ is returned. These are the cases:
 * 1. \f$\bot\f$: \f$\neg P_k(\hat x,x_\varepsilon)\Longrightarrow\hat x+x_\varepsilon>2^k=2\cdot2^{k-1}>2(\hat x-x_\varepsilon)\Longrightarrow x_\varepsilon>\hat x/3>|x|/3\f$.
 * 2. \f$T\f$: \f$P_k(\hat x,x_\varepsilon)\wedge\hat x<2^k\Longrightarrow2^k>\hat x+x_\varepsilon>|x|\f$
 * 3. \f$F\f$: \f$P_k(\hat x,x_\varepsilon)\wedge\hat x\geq2^k\Longrightarrow\hat x-x_\varepsilon\geq 2^{k-1}\Longrightarrow|x|\geq 2^{k-1}-2^e\f$ for \f$\hat x=m\cdot2^e\f$.
 *
 * \param x real number to bound the absolute value of
 * \param k base-2 logarithm of the bound
 * \return \f$\text{bound}(x,k)=\begin{cases}
 *    \bot,&\neg P_k(\hat x,x_\varepsilon)\Longrightarrow x_\varepsilon>|x|/3
 * \\ T,&P_k(\hat x,x_\varepsilon)\wedge\hat x<2^k\Longrightarrow |x|<2^k
 * \\ F,&P_k(\hat x,x_\varepsilon)\wedge\hat x\not<2^k\Longrightarrow |x|\geq 2^{k-1}-2^e
 * \end{cases}\f$
 * \todo According to documentation, this function should compute
 * \f$\begin{cases}
 *    T,&|x|\leq 2^k
 * \\ F,&|x|\geq 2^{k-2}
 * \\ \bot,&\text{otherwise}
 * \end{cases}\f$
 */
LAZY_BOOLEAN bound(const REAL & x, const int k)
{
	if (!x.value) {
		REAL y(x);
		return bound(y.mp_conv(), k);
	}
	sizetype lowsize, ksize, highsize;
	sizetype_set(ksize, 1, k);
	lowsize = sizetype_add_power2(x.error, k - 1);
	sizetype_add(highsize, x.vsize, x.error);
	if (sizetype_less(x.vsize, lowsize) && sizetype_less(ksize, highsize)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) in bounding "
		                "by 2^(%d) for argument of size  %d*2^(%d)\n",
		             x.error.mantissa, x.error.exponent, k,
		             x.vsize.mantissa, x.vsize.exponent);
		return LAZY_BOOLEAN::BOTTOM;
	}
	return (sizetype_less(x.vsize, ksize));
}


/*! \ingroup debug */
void REAL::rcheck(int n) const
{
	if (!value) {
		cerr << "Value: (" << std::setprecision(n) << std::setw(n)
		     << dp.lower_pos << ";" << std::setprecision(n)
		     << std::setw(n) << -dp.upper_neg << ")\n";
	} else {
		cerr << "Value: ";
		char * c = MP_swrite(value, n);
		cerr << c;
		free(c);
		cerr << ", Size:  " << vsize.mantissa << "*2^("
		     << vsize.exponent << ")\n\n";
		cerr << ", Error: " << error.mantissa << "*2^("
		     << error.exponent << ")\n\n";
	}
}

void REAL::adderror(sizetype nerror)
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().adderror(nerror);
		*this = y;
	} else {
		sizetype_inc((*this).error, nerror);
	}
}

void REAL::seterror(sizetype nerror)
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().seterror(nerror);
		*this = y;
	} else {
		(*this).error = nerror;
	}
}

void REAL::geterror(sizetype & nerror) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().geterror(nerror);
	} else {
		nerror = (*this).error;
	}
}

void REAL::getsize(sizetype & nsize) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().getsize(nsize);
	} else {
		nsize = (*this).vsize;
	}
}

void REAL::to_formal_ball(DYADIC & d, sizetype & nerror) const
{
	if (!value) {
		REAL y(*this);
		y.mp_conv().geterror(nerror);
		MP_duplicate_wo_init(y.value, d.value);
	} else {
		nerror = (*this).error;
		MP_duplicate_wo_init(this->value, d.value);
	}
}

REAL::REAL(const DYADIC & y)
{
	MP_duplicate_w_init(y.value, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

REAL::REAL(const std::string &s) : REAL(s.c_str()) {}
REAL::REAL(const char        *s) : REAL(atoREAL(s)) {}

INTEGER REAL::as_INTEGER() const
{
	if (!this->value) {
		return this->mp_conv().as_INTEGER();
	}
	MP_int_type result, value;
	if (get_cached(result)) {
		MP_int_duplicate_w_init(result, value);
		return { value, INTEGER::move_t{} };
	}

	sizetype psize;
	sizetype_set(psize, 1, -4);
	REAL y = (*this);
	if (MP_sign(y.value) == 1)
		y = y + REAL(0.5);
	else
		y = y - REAL(0.5);
	y.mp_conv();
	if (sizetype_less(psize, y.error)) {
		iRRAM_DEBUG2(1, "insufficient precision %d*2^(%d) converting "
		                "to integer\n",
		             this->error.mantissa, this->error.exponent);
		iRRAM_REITERATE(-y.error.exponent);
	}
	MP_int_init(value);
	MP_mp_to_INTEGER(y.value, value);

	if (actual_stack().inlimit == 0) { /* TODO: ... or duplicate the MP_*_types */
		MP_int_duplicate_w_init(value, result);
		put_cached(result);
	}
	return { value, INTEGER::move_t{} };
}

// conversion to type REAL from smaller types

REAL::REAL(const RATIONAL & r)
{
	INTEGER numi, deni;

	MP_rat_get_numerator(r.value, numi.value);
	MP_rat_get_denominator(r.value, deni.value);

	REAL result = REAL(numi) / REAL(deni);
	result.mp_conv();
	*this = std::move(result);
}

REAL::REAL(const INTEGER & y)
{
	MP_init(value);
	MP_INTEGER_to_mp(y.value, value);
	sizetype_exact(error);
	MP_getsize(value, vsize);
}

/*! \brief Converts a base-10 string in scientific notation to \ref REAL.
 *
 * If \f$p\f$ is the current working precision of iRRAM, \f$k\f$ the number of
 * digits in the integer part, \f$z\f$ the number of leading zero-digits
 * (including the fractional part) and \f$g\f$ the exponent, and the represented
 * value is not zero, then the result has a relative precision of roughly
 * \f$(g+k-z)\log_2(10)-p\f$ bits, which corresponds to an absolute accuracy of
 * \f$2^p\f$.
 *
 * \param[in] s pointer to base-10 string; format: WSDFE where
 *              * W = optional whitespace; it is discarded
 *              * S = optional sign
 *              * D = optional string of digits `0`..`9`
 *              * F = optional: `.` followed by optional string of digits
 *                `0`..`9`
 *              * E = optional: `e` or `E` followed by an optional sign
 *                followed by a string of digits `0`..`9`
 *              * DF must be non-empty and not just contain `.`
 * \param[out] endptr when non-NULL, the pointer to the end of the represented
 *                    number is stored in `*endptr`
 * \return a REAL object accurate to the current working precision.
 *
 * \par Proof
 * The precision depends only on DF and E; let \f$x_k\ldots x_{-n}=DF[1..|F|]\f$
 * and \f$z\in\mathbb N:x[1..z]=\texttt 0^z\f$. To represent zero up to any
 * accuracy, only 1 bit is required. So let \f$x\neq0\f$ and \f$b\cdot2^c\f$
 * with \f$b\in[2^{-1};1)\f$ be the result. Then the error is
 * \f[ (*)=|10^g\sum_{i=-n}^{k-z} x_i\cdot 10^i-2^cb|
 *    =|2^{\overbrace{(g+k-z+1)\log_2(10)}^{=:q}}
 *     \underbrace{\sum_{i=-n-(k-z+1)}^{-1}x_{i+k-z+1}\cdot10^i}_{=:x'\in[10^{-1};1)}
 *    -2^cb|
 * \f]
 * Now, MPFR's mpfr_strtofr() guarantees for the exact input
 * \f$2^qx'=:2^{\overline q}\overline b\f$ with \f$\overline b\in[2^{-1};1)\f$
 * that the error is
 * \f$|2^{\overline q}\overline b-2^cb|<2^{\overline q-m}\f$, therefore
 * \f[(*)=|2^qx'-2^{\overline q}\overline b+2^{\overline q}\overline b-2^cb|
 *       <|2^qx'-2^{\overline q}\overline b|+2^{\overline q-m}=2^{\overline q-m}
 * \f]
 * Since \f$2^{\overline q}/2\leq 2^{\overline q}\overline b=2^qx'<2^q\f$,
 * the error is bounded, \f$(*)<2^p\f$, when
 * \f$\overline q-p<m\leq q+1-p<\lceil(g+k-z+1)\log_2(10)\rceil+1-p\leq m'\f$.
 * The exact precision is \f$\max(10,m')\f$ where \f$m'=1+\begin{cases}
 *  1&z=k+n+1\\
 *  \lfloor((g+k-z+1)\cdot 10+2)/3+1\rfloor-p&\text{otherwise}
 * \end{cases}\f$
 */
REAL strtoREAL2(const char *s, char **endptr)
{
	const char *t = s;
	const char *dk, *d0, *f1, *fn;
	int k, z = 0, n;
	long g = 0;

	while (isblank(*t)) t++;
	if (*t == '+' || *t == '-') t++;

	dk = t;
	while (*t == '0') { z++; t++; }
	while ('0' <= *t && *t <= '9') t++;
	d0 = t-1;
	k = d0 - dk;
	if (*t == '.') t++;
	f1 = t;
	if (z > k) while (*t == '0') { z++; t++; }
	while ('0' <= *t && *t <= '9') t++;
	fn = t-1;
	n = fn - f1 + 1;

	if (*t == 'e' || *t == 'E')
		g = strtol(t+1, (char **)&t, 10);

	/* d[k] ... d[0] . f[-1] .. f[-n] */

	stiff code;

	int p = actual_stack().actual_prec;
	int m = z < k+n+1
	      ? /* at least one non-zero digit */
	        ((g+k-z+1)*10+2)/3+1 - p /* (g+k-z+1)*log_2(10)+1 - p */
	      : 1;

	iRRAM_DEBUG2(2,"strtoREAL2(%s): k: %d, n: %d, z: %d, g: %ld, p: %d, m: %d\n",
	             s, k, n, z, g, p, m);

	char *mpfr_endptr;

	MP_type value;
	MP_init(value);
	mpfr_set_prec(value, max(10, m+1));
	int r = mpfr_strtofr(value, s, &mpfr_endptr, 10, MPFR_RNDN);
	ext_mpfr_remove_trailing_zeroes(value);

	assert(mpfr_endptr == t);
	if (endptr)
		*endptr = (char *)t;

	return REAL(value, r ? sizetype_power2(p) : sizetype_exact());
}

REAL strtoREAL(const char * s, char ** endptr)
{
	stiff code;
	int exp = 0;
	int sign = 1;
	REAL y;
	REAL ten = 10;
	*endptr = const_cast<char *>(s);

	while (**endptr == '0')
		*endptr += 1;

	if (**endptr == '-') {
		sign = -1;
		*endptr += 1;
	} else if (**endptr == '+') {
		sign = 1;
		*endptr += 1;
	}

	while (**endptr >= '0' && **endptr <= '9') {
		y = ten * y + REAL(sign * (**endptr - '0'));
		*endptr += 1;
	}

	if (**endptr == '.')
		*endptr += 1;

	while (**endptr >= '0' && **endptr <= '9') {
		y = ten * y + REAL(sign * (**endptr - '0'));
		exp -= 1;
		*endptr += 1;
	}

	if (**endptr == 'E' || **endptr == 'e') {
		*endptr += 1;
		exp += strtol(*endptr, endptr, 10);
	}

	if (exp != 0)
		y *= power(ten, exp);

	return y;
}

REAL atoREAL(const char * s)
{
	char * dummy;
	return strtoREAL(s, &dummy);
}

REAL modulo(const REAL & x, const REAL & y) { return x - round2(x / y) * y; }

REAL power(const REAL & x, const REAL & y) { return exp(log(x) * y); }

REAL power(const REAL & x, int n)
{
	if (n == 0)
		return 1;
	if (n == 1)
		return x;
	if (n == 2)
		return square(x);
	//   stiff_begin();
	REAL y = 1;
	REAL xc = x;
	if (n < 0) {
		xc = y / x;
		n = -n;
	}
	if (n == 1) {
		//     stiff_end();
		return xc;
	}
	for (int k = n; k > 0; k = k / 2) {
		if (k % 2 == 1)
			y *= xc;
		if (k == 1)
			break;
		xc = square(xc);
	}
	//   stiff_end();
	return y;
}

// maximum without using of internal representation of LAZY_BOOLEAN
// REAL maximum (const REAL& x, const REAL& y){
//    single_valued code;
//    LAZY_BOOLEAN larger = ( x > y );
//    switch ( choose ( larger, !larger, TRUE ) ){
//    case 1: return x;
//    case 2: return y;
//    case 3: return (x+y+abs(x-y))/2;
//    }
// };

// maximum, using of internal representation of LAZY_BOOLEAN
REAL mp_maximum(const REAL & x, const REAL & y)
{
	LAZY_BOOLEAN larger;
	{
		single_valued code;
		larger = (x > y);
	}
	if (larger.value == true)
		return x;
	if (larger.value == false)
		return y;
	return (x + y + abs(x - y)) / 2;
}

// minimum, using of internal representation of LAZY_BOOLEAN
REAL mp_minimum(const REAL & x, const REAL & y)
{
	LAZY_BOOLEAN larger;
	{
		single_valued code;
		larger = (x > y);
	}
	if (larger.value == true)
		return y;
	if (larger.value == false)
		return x;
	return (x + y - abs(x - y)) / 2;
}

//********************************************************************************
// Absolute value of vector in Euclidean space
//********************************************************************************

REAL abs(const std::vector<REAL>& x)
{
	unsigned int n=x.size();
	REAL sqrsum=0;
	for (unsigned i=0;i<n;i++) {
		sqrsum += square(x[i]);
	}
	return sqrt(sqrsum);
}

// Conversions from REAL to DYADIC with absolute precision
DYADIC REAL::as_DYADIC(const int p) const { return approx(*this, p); }

DYADIC REAL::as_DYADIC() const { return approx(*this, DYADIC::getprec()); }

// Conversion from REAL to double with a relative precision of p bits
double REAL::as_double(const int p) const
{
	if (bound(*this, -1150))
		return 0.0;
	int s = size(*this);
	DYADIC d = approx(*this, s - p - 2);
	return MP_mp_to_double(d.value);
}

/*****************************************/
// module function (will be a template later...)

int module(REAL (*f)(const REAL&),const REAL& x, int p){
// Semantics: If m=module(f,x,p), then
//    |x-z| <=2^m implies  |f(x)-f(z)| <= 2^p 

// If we are able to approximate f(x) by a DYADIC number d with an error of <=2^{p-1},
// then for any z in the argument interval of x, f(z) must differ by at most 
//  2^{p-1} from d, hence |f(x)-f(z)|<=2^p

  int result;
  if (get_cached(result))
    return result;

  DYADIC d;
  REAL x_copy=x;

  
  sizetype argerror,testerror;
  
  x_copy.geterror(argerror);
  testerror = sizetype_power2(argerror.exponent);
  x_copy.adderror(testerror);
  {
    single_valued code;
    d=approx(f(x_copy),p-1); 
  }
// At this line, we are sure that x_copy (and so also x) is precise enough to allow 
// the computation of f(x), even with a slightly increased error of the argument.

// We now try to find the smallest p_arg such that the evaluation of f(x+- 2^p_arg) 
// is possible up to an error of at most  2^{p-1}

// To do this, we start with p_arg=p.
// If this is successfull, we increase the value of p_arg until the first failure
// It it is not, then we decrease until the first success...
  int direction=0,p_arg=p;
  bool try_it=true;

  while (try_it) {
    testerror = sizetype_power2(p_arg);
    x_copy.seterror(argerror);
    x_copy.adderror(testerror);
    bool fail = false;
  if ( iRRAM_unlikely(state->debug > 0 ) ) {
   sizetype x_error;
   x_copy.geterror(x_error);
  iRRAM_DEBUG2(1,"Testing module: 1*2^%d + %d*2^%d\n",p_arg,argerror.mantissa,argerror.exponent);
  iRRAM_DEBUG2(1,"argument error: %d*2^%d\n",x_error.mantissa,x_error.exponent);
  }
  try { 
      single_valued code;
      REAL z=f(x_copy);
      if ( iRRAM_unlikely(state->debug > 0 ) ) {
        sizetype z_error;
        z.geterror(z_error);
        iRRAM_DEBUG2(1,"Module yields result %d*2^%d\n",z_error.mantissa,z_error.exponent);
      }
      d=approx(z,p-1); 
	}
    catch ( Iteration it)  { fail=true; }
    switch ( direction ) {
      case 0:;
        if ( fail ) direction=-1; else direction=1;
	p_arg+=direction;
      break;
      case 1:;
        if ( fail ) { try_it=false; p_arg -=direction; }
	else { p_arg += direction; };
      break;
      case -1:;
        if ( fail ) { p_arg +=direction; }
	else { try_it=false; };
      break;    
    }
    }
  iRRAM_DEBUG2(1,"Modules resulting in p_arg=%d\n",p_arg);
  
  testerror = sizetype_power2(p_arg);
  argerror += testerror;

  while (argerror.mantissa>1) {
    argerror.mantissa=argerror.mantissa>>1;
    argerror.exponent+=1;
  }
  
  result=argerror.exponent;
  put_cached(result);
  return result;

}

} // namespace iRRAM
