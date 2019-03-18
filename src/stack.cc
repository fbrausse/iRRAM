/*

stack.cc -- basic file for the iterations in the iRRAM library
 
Copyright (C) 2001-2006 Norbert Mueller
 
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

#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/core.h>
#include <iRRAM/cache.h>
#include <iRRAM/DYADIC.h>
#include <iRRAM/INTEGER.h>

//*************************************************************************************
// runtime identification of the iRRAM version, cf. iRRAM_version.h
//*************************************************************************************

extern "C" {
const char *iRRAM_VERSION_rt = iRRAM_VERSION_ct;
}

namespace iRRAM {

// iRRAM_TLS bool iRRAM_COMPARE_exact=true; /* unused */
// iRRAM_TLS int iRRAM_COMPARE_precision=-60; /* unused */

mv_cache::mv_cache() = default;
mv_cache::~mv_cache() = default;

} // namespace iRRAM

// 
