/*

RATIONAL.cc -- routines for RATIONAL data type
 
Copyright (C) 2001-2003 Norbert Mueller, Tom van Diessen
Copyright     2016      Franz Brausse
 
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


/*
Changelog: (initial version by Tom, all modifications by Norbert and Franz)

  2016-11-28  Remove explicit casts from INTEGER (use constructor instead)
              Allow for copy-elision / moves
              Optimize operator+=(RATIONAL,int) and operator-(RATIONAL)

  2003-08-29  Addition of functions x+i, x-i, x*i, x/i and i+x etc
              for i of simple type "int"

  2001-07-19  Initial version from Tom's diploma thesis

*/

#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/RATIONAL.h>
#include <iRRAM/INTEGER.h>

namespace iRRAM {

//****************************************************************************************
// CONSTRUCTORS
// IMPORTANT: Objects MUST be initialized before being used.
//****************************************************************************************

//****************************************************************************************
// Constructing RATIONAL from scratch
//****************************************************************************************

RATIONAL::RATIONAL(const INTEGER& i,const INTEGER& j){
  MP_rat_init(value);
  MP_INTINTEGER_to_RATIONAL(i.value,j.value,value);
}

RATIONAL::RATIONAL(int i, int j){
  MP_rat_init(value);
  if ( j >= 0) {
	MP_intint_to_RATIONAL(i,(unsigned int)(j),value);
	} else {
	INTEGER ii(i);
	INTEGER jj(j);
	MP_INTINTEGER_to_RATIONAL(ii.value,jj.value,value);
	}
}

RATIONAL::RATIONAL(const INTEGER& i){
  MP_rat_init(value);
  MP_INTEGER_to_RATIONAL(i.value,value);
}

RATIONAL::RATIONAL(int i) {
  MP_rat_init(value);
  MP_int_to_RATIONAL(i,value);
}

RATIONAL::RATIONAL(const RATIONAL& y){
  MP_rat_duplicate_w_init(y.value, value);
}


//****************************************************************************************
// Constructing RATIONAL from double, the result is NOT rounded
//****************************************************************************************

RATIONAL::RATIONAL(double d){
  MP_rat_init(value);
  MP_double_to_RATIONAL(d,value);
}

//****************************************************************************************
// Constructing RATIONAL from string, the string must be in decimal base
//****************************************************************************************

RATIONAL::RATIONAL(const char* s){
  MP_rat_init(value);
  MP_string_to_RATIONAL(s,value);
}

//******************************************************************************
// Copy assignment
// left side (this) is already initialized
//******************************************************************************

RATIONAL & RATIONAL::operator=(RATIONAL y)
{
	using std::swap;
	swap(value, y.value);
	return *this;
}

RATIONAL& RATIONAL::operator = (int y){
  MP_int_to_RATIONAL(y,value);
  return (*this);
}

//**************************************************************/
// Destructor
// Destroying the object and freeing space occupied by it
//******************************************************************/

RATIONAL::~RATIONAL(){
  if (value) MP_rat_clear(value);
}

//****************************************************************************************
// Returns the sign of RATIONAL objects (-1/0/1): 
// sign(i)= -1, iff i<0,   sign(i)=1 iff i>0,  and sign(0)=0
//****************************************************************************************
/*! \ingroup maths */
int sign(const RATIONAL& x)
{
  return MP_rat_sign(x.value);
}

//****************************************************************************************
// Shift
// defines shifting function if none exists in the backend
//****************************************************************************************

RATIONAL scale(RATIONAL x, int n)
{
	MP_rat_shift(x.value,x.value,n);
	return x;
}


//*********************************************************************/
// Addition
//**********************************************************************/

RATIONAL& operator += (RATIONAL& x, const RATIONAL& y)
{
  MP_rat_add(x.value,y.value,x.value);
  return x;
}

RATIONAL & operator+=(RATIONAL &x, int y)
{
	MP_rat_add_si_inplace(y,x.value);
	return x;
}

//******************************************************************************
// Subtraction
//******************************************************************************

RATIONAL operator-(int x, RATIONAL y)
{
	MP_rat_neg(y.value, y.value);
	y += x;
	return y;
}

RATIONAL& operator -= (RATIONAL& x, const RATIONAL& y){
  MP_rat_sub(x.value,y.value,x.value);
  return x;
}

//******************************************************************************
// Negation
//******************************************************************************

RATIONAL operator-(RATIONAL x)
{
	MP_rat_neg(x.value, x.value);
	return x;
}

//******************************************************************************
// Multiplication
//******************************************************************************

RATIONAL& operator *= (RATIONAL& x, const RATIONAL& y){
  MP_rat_mul(x.value,y.value,x.value);
  return x;
}

RATIONAL & operator*=(RATIONAL &x, const int n)
{
	MP_rat_mul_si(x.value, n, x.value);
	return x;
}

//******************************************************************************
// Division
//******************************************************************************

RATIONAL operator / (int x, RATIONAL y)
{
  MP_rat_si_div(x,y.value,y.value);
  return y;
}

RATIONAL& operator /= (RATIONAL& x, const RATIONAL& y){
  MP_rat_div(x.value,y.value,x.value);
  return x;
}

RATIONAL& operator /= (RATIONAL& x, const int n)
{
	MP_rat_div_si(x.value,n,x.value);
	return x;
}


//****************************************************************************************
// Absolute value: |x|
//****************************************************************************************

/*! \ingroup maths */
RATIONAL abs(RATIONAL x)
{
	MP_rat_abs(x.value,x.value);
	return x;
}

INTEGER denominator (const RATIONAL& x){
  MP_int_type zvalue;
  MP_int_init(zvalue);
  MP_rat_get_denominator(x.value,zvalue);
  return { zvalue, INTEGER::move_t{} };
}

INTEGER numerator (const RATIONAL& x){
  MP_int_type zvalue;
  MP_int_init(zvalue);
  MP_rat_get_numerator(x.value,zvalue);
  return { zvalue, INTEGER::move_t{} };
}

RATIONAL power(RATIONAL x, unsigned n)
{
	MP_rat_power(x.value, n, x.value);
	return x;
}

//****************************************************************************************
// Comparators
// returns boolean value 1 if x<y, 0 otherwise
//****************************************************************************************

bool operator < (const RATIONAL& x, const RATIONAL& y){
  return (MP_rat_compare(x.value,y.value) < 0 );
}

//****************************************************************************************
// Equal to: x == y
// returns boolean value 1 if x==y, 0 otherwise
//****************************************************************************************

bool operator == (const RATIONAL& x, const RATIONAL& y){
  return (MP_rat_equal(x.value,y.value));
}


//*****************************************************************************
// Function swrite
// string function for RATIONAL
// writes rational into string
//*****************************************************************************

std::string swrite(const RATIONAL& x, const int w){
     char* erg= MP_rat_swritee(x.value,w);
     std::string result=erg;
     free(erg);
     return result;
}

//****************************************************************************************
// swrite: writes RATIONAL to string
// 1. argument: RATIONAL
//****************************************************************************************

std::string swrite(const RATIONAL& x){
     char* erg= MP_rat_sprintf(x.value);
     std::string result=erg;
     free(erg);
     return result;  
}

} // namespace iRRAM
