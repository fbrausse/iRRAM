/*

LAZY_BOOLEAN.cc -- implementation of lazy booleans for the iRRAM library
 
Copyright (C) 2003 Norbert Mueller
 
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
#include <cstring>

#include <algorithm>

#include <iRRAM/LAZYBOOLEAN.h>
#include <iRRAM/cache.h>
#include <iRRAM/STREAMS.h> /* iRRAM_DEBUG* */

namespace iRRAM {

LAZY_BOOLEAN::operator bool() const {
  bool result;
  if (!get_cached(result)){

    if ( value <= LAZY_BOOLEAN::BOTTOM ){
      iRRAM_DEBUG1(1,"lazy boolean values BOTTOM leading to iteration\n");
      iRRAM_REITERATE(0);
    }

    result=value;
    put_cached(result);
  }
  return result;
}

int check( const LAZY_BOOLEAN& lb) {
  int result;
  if (!get_cached(result)) {
    result=lb.value;
    put_cached(result);
  }
  return result;
}

std::size_t choose(std::initializer_list<LAZY_BOOLEAN> x)
{
	using std::find_if;

	std::size_t result;
	if (get_cached(result))
		return result;

	auto is_true = [](const LAZY_BOOLEAN &p)
	               { return p.value == true; };
	auto is_bot  = [](const LAZY_BOOLEAN &p)
	               { return p.value == LAZY_BOOLEAN::BOTTOM; };

	auto true_pos = find_if(begin(x), end(x), is_true);
	if (true_pos != end(x)) {
		result = 1 + (true_pos - begin(x));
	} else if (find_if(begin(x), end(x), is_bot) != end(x)) {
		iRRAM_DEBUG1(1,"choose(init-list): lazy boolean value BOTTOM "
		               "leading to iteration\n");
		iRRAM_REITERATE(0);
	} else {
		result = 0;
	}

	put_cached(result);
	return result;
}

int choose(const LAZY_BOOLEAN& x1,
           const LAZY_BOOLEAN& x2,
           const LAZY_BOOLEAN& x3,
           const LAZY_BOOLEAN& x4,
           const LAZY_BOOLEAN& x5,
           const LAZY_BOOLEAN& x6)
{
  int result=0;
  if (get_cached(result))
    return result;

  int minvalue=false;
  if (x1.value == 1 )result=1; else
   if (x2.value == 1 )result=2; else
    if (x3.value == 1 )result=3; else
     if (x4.value == 1 )result=4; else
      if (x5.value == 1 )result=5; else
       if (x6.value == 1 )result=6; else
        if (x1.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM; else
         if (x2.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM; else
          if (x3.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM; else
           if (x4.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM; else
            if (x5.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM; else
             if (x6.value == LAZY_BOOLEAN::BOTTOM )minvalue=LAZY_BOOLEAN::BOTTOM;

  if ( minvalue == LAZY_BOOLEAN::BOTTOM ){
    iRRAM_DEBUG1(1,"lazy boolean value BOTTOM leading to iteration\n");
    iRRAM_REITERATE(0);
  }

  put_cached(result);
  return result;
}

LAZY_BOOLEAN operator && 
   (const LAZY_BOOLEAN& x, const LAZY_BOOLEAN& y) noexcept
   {
     if (x.value==1 && y.value==1) return 1;
     if (x.value==0 || y.value==0) return 0;
     return LAZY_BOOLEAN::BOTTOM;
   } 

LAZY_BOOLEAN operator || 
   (const LAZY_BOOLEAN& x, const LAZY_BOOLEAN& y) noexcept
   {
     if (x.value==1 || y.value==1) return 1;
     if (x.value==0 && y.value==0) return 0;
     return LAZY_BOOLEAN::BOTTOM;
   } 

LAZY_BOOLEAN operator !
   (const LAZY_BOOLEAN& x) noexcept
   {
     if (x.value==1) return 0;
     if (x.value==0) return 1;
     return LAZY_BOOLEAN::BOTTOM;
   } 

} // namespace iRRAM
