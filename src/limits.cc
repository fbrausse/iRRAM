/*

limit.cc -- Templates for general limits, lipschitz limits ...
            for the iRRAM library

Copyright (C) 2001-2005 Norbert Mueller

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
Changelog: (initial version by Norbert)

  2001-09-11 correction of ordinary limits due to
             moving from longjump to exception as iteration mechanism

*/

#include <iRRAM/limit_templates.h>
#include <iRRAM/COMPLEX.h>
#include <iRRAM/REALMATRIX.h>
#include <iRRAM/FUNCTION.h>

namespace iRRAM {

/*! \addtogroup limits
 * @{ */

REAL limit_lip (REAL (*f)(int,const REAL&),
            int lip_value,
	    bool (*on_domain)(const REAL&),
            const REAL& x)
{
  if ( on_domain(x) != true ) iRRAM_REITERATE(0);

  limit_computation env;

  REAL x_new,lim;
  sizetype x_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);

  limit_debug("starting limit_lip1");

  while (1) {
    try{
    iRRAM_DEBUG2(2,"trying to compute limit_lip1 with precision %d...\n",actual_stack().actual_prec);
    lim=f(env.saved_prec(),x_new);
    if (lim.error.exponent > env.saved_prec()) {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip1 too imprecise, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lim.error.mantissa, lim.error.exponent);
      break;
    }}
    catch ( Iteration it){
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip1 failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } }
  lim.adderror(sizetype_add_power2(x_error << lip_value, env.saved_prec()));
  iRRAM_DEBUG2(2,"end of limit_lip1 with error %d*2^(%d)\n"
                 "  error of argument: %d*2^(%d)\n",
                 lim.error.mantissa,lim.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lim;
}

REAL limit_lip (REAL (*f)(int,const REAL&),
            int (*lip_bound)(const REAL&),
            const REAL& x)
{
  int lip_value;
  {
    single_valued code;
    lip_value=lip_bound(x);
  }
  
  limit_computation env;
  REAL x_new;
  REAL lim;
  sizetype x_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);

  limit_debug("starting limit_lip1");

  while (1) {
     try{
      iRRAM_DEBUG2(2,"trying to compute limit_lip1 with precision %d...\n",actual_stack().actual_prec);
    lim=f(env.saved_prec(),x_new);
    if (lim.error.exponent > env.saved_prec()) {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip1 too imprecise, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lim.error.mantissa, lim.error.exponent);
      break;
    }}
    catch ( Iteration it){
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip1 failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } }
  lim.adderror(sizetype_add_power2(x_error << lip_value, env.saved_prec()));
  iRRAM_DEBUG2(2,"end of limit_lip1 with error %d*2^(%d)\n"
                 "  error of argument: %d*2^(%d)\n",
                 lim.error.mantissa,lim.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lim;
}

REAL limit_lip     (REAL (*f)(int, const REAL&, const REAL&),
         int lip,
         bool (*on_domain)(const REAL&,const REAL&),
         const REAL& x,
         const REAL& y)
{
  if ( on_domain(x,y) != true ) iRRAM_REITERATE(0);

  limit_computation env;

  REAL x_new,y_new,lim;
  sizetype x_error,y_error;

  x_new=x; x_new.geterror(x_error); assert(x_new.value); sizetype_exact(x_new.error);
  y_new=y; y_new.geterror(y_error); assert(y_new.value); sizetype_exact(y_new.error);

  limit_debug("starting limit_lip2");

  while (1) {
    try {
      iRRAM_DEBUG2(2,"trying to compute limit_lip2 with precision %d...\n",actual_stack().actual_prec);
      lim=f(env.saved_prec(),x_new,y_new);
     if (lim.error.exponent > env.saved_prec()) {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip2 too imprecise, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lim.error.mantissa, lim.error.exponent);
      break;
    }}
    catch ( Iteration it) {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip2 failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    }}
  lim.adderror(sizetype_add_power2((x_error + y_error) << lip, env.saved_prec()));
  iRRAM_DEBUG2(2,"end of limit_lip2 with error %d*2^(%d)\n"
                 "  error of argument 1: %d*2^(%d)\n"
                 "  error of argument 2: %d*2^(%d)\n",
                 lim.error.mantissa,lim.error.exponent,
                 x_error.mantissa,x_error.exponent,
                 y_error.mantissa,y_error.exponent);
  return lim;
}

REAL lipschitz (REAL (*f)(const REAL&),
            int lip,
	    bool (*on_domain)(const REAL&),
            const REAL& x)
{
  if ( on_domain(x) != true ) iRRAM_REITERATE(0);

  REAL x_new,lip_result;
  sizetype x_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);
  {
    single_valued code;
  iRRAM_DEBUG1(2,"starting lipschitz1 ...\n");
  lip_result=f(x_new);
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  }
  lip_result.adderror(x_error << lip);
  iRRAM_DEBUG2(2,"end of lipschitz_1 with error %d*2^(%d)\n"
                 "  for argument with error %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lip_result;
}

REAL lipschitz (REAL (*f)(const REAL&),
            REAL (*lip_f)(const REAL&),
	    bool (*on_domain)(const REAL&),
            const REAL& x)
{
  if ( on_domain(x) != true ) iRRAM_REITERATE(0);

  REAL x_new,lip_result,lip_bound;
  sizetype x_error;

  {
    limit_computation env(0, stiff::rel{});
  iRRAM_DEBUG1(2,"starting lipschitz1b ...\n");

// for the computation of the Lipschitz bound, we work with
// reduced precision:
  {
    relaxed rel;
    lip_bound=lip_f(x);
  }
  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);


  bool try_it=true;
  while (try_it) {
  try { try_it=false;
        lip_result=f(x_new); }
  catch ( Iteration it)  { try_it=true;
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip2 failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    }
  }
  }
  
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  lip_result.adderror((lip_bound.getsize() + lip_bound.geterror()) * x_error);
  iRRAM_DEBUG2(2,"end of lipschitz_1b with error %d*2^(%d)\n"
                 "  for argument with error %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lip_result;
}

REAL lipschitz (REAL (*f)(const REAL&),
            REAL (*lip_f)(const REAL&),
            const REAL& x)
{
  REAL x_new,lip_result,lip_bound;
  sizetype x_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);

  {
    single_valued code;
  iRRAM_DEBUG1(2,"starting lipschitz1a ...\n");
  lip_result=f(x_new);

// for the computation of the Lipschitz bound, we work with
// reduced precision:
  {
    relaxed rel;
    lip_bound=lip_f(x);
  }
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  }
  lip_result.adderror((lip_bound.getsize() + lip_bound.geterror()) * x_error);
  iRRAM_DEBUG2(2,"end of lipschitz_1a with error %d*2^(%d)\n"
                 "  for argument with error %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lip_result;
}

REAL lipschitz (REAL (*f)(int, const REAL&),
            int lip,
	    bool (*on_domain)(int k,const REAL&),
            int k,
            const REAL& x)
{
  if ( on_domain(k,x) != true ) iRRAM_REITERATE(0);
  REAL x_new,lip_result;
  sizetype x_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);
  {
    single_valued code;
  iRRAM_DEBUG1(2,"starting lipschitz1 ...\n");
  lip_result=f(k,x_new);
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  }
  lip_result.adderror(x_error << lip);
  iRRAM_DEBUG2(2,"end of lipschitz_1 with error %d*2^(%d)\n"
                 "  error of argument: %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent);
  return lip_result;
}

REAL lipschitz (REAL (*f)(const REAL&, const REAL&),
            int lip,
	    bool (*on_domain)(const REAL&,const REAL&),
            const REAL& x,
            const REAL& y)
{
  if ( on_domain(x,y) != true ) iRRAM_REITERATE(0);
  REAL x_new,y_new,lip_result;
  sizetype x_error,y_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);
  y_new=y;
  y_new.geterror(y_error);
  assert(y_new.value);
  sizetype_exact(y_new.error);
  {
    single_valued code;
  iRRAM_DEBUG1(2,"starting lipschitz2 ...\n");
  lip_result=f(x_new,y_new);
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  }
  lip_result.adderror((x_error + y_error) << lip);
  iRRAM_DEBUG2(2,"end of lipschitz_2 with error %d*2^(%d)\n"
                 "  error of argument x: %d*2^(%d)\n"
                 "  error of argument y: %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent,
                 y_error.mantissa,y_error.exponent);
  return lip_result;
}

REAL lipschitz (REAL (*f)(int, const REAL&, const REAL&),
            int lip,
	    bool (*on_domain)(int k,const REAL&,const REAL&),
            int k,
            const REAL& x,
            const REAL& y)
{
  if ( on_domain(k,x,y) != true ) iRRAM_REITERATE(0);
  REAL x_new,y_new,lip_result;
  sizetype x_error,y_error;

  x_new=x;
  x_new.geterror(x_error);
  assert(x_new.value);
  sizetype_exact(x_new.error);
  y_new=y;
  y_new.geterror(y_error);
  assert(y_new.value);
  sizetype_exact(y_new.error);
  x_new.geterror(x_error);
  {
    single_valued code;
  iRRAM_DEBUG1(2,"starting lipschitz2 ...\n");
  lip_result=f(k,x_new,y_new);
  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  }
  lip_result.adderror(x_error << lip);
  lip_result.adderror(y_error << lip);
  iRRAM_DEBUG2(2,"end of lipschitz_2 with error %d*2^(%d)\n"
                 "  error of argument x: %d*2^(%d)\n"
                 "  error of argument y: %d*2^(%d)\n",
                 lip_result.error.mantissa,lip_result.error.exponent,
                 x_error.mantissa,x_error.exponent,
                 y_error.mantissa,y_error.exponent);
  return lip_result;
}

REAL limit_hint    (REAL (*f)(int, const REAL&),
                    int hint,
                    const REAL& x)
{
  limit_computation env;

  REAL lim,limnew;
  sizetype limnew_error;
  sizetype lim_error;
  int hintcopy=hint;
  int success=0;

  int element=max(x.geterror().exponent,env.saved_prec());

  iRRAM_DEBUG1(2,"starting limit_hint1...\n");

  while (1) {
    try {
    iRRAM_DEBUG2(2,"trying to compute limit_hint1 with precicion 2^(%d)...\n",element);
    limnew=f(element,x);
    limnew_error = sizetype_add_power2(limnew.geterror(), element);
    if ( (! success) || sizetype_less(limnew_error,lim_error) ) {
      lim=limnew;
      lim_error=limnew_error;
      iRRAM_DEBUG2(2,"getting result with error %d*2^(%d)\n",
               lim_error.mantissa, lim_error.exponent);
      } else {
      iRRAM_DEBUG1(2,"computation successful, but no improvement\n");
      hintcopy=2*hintcopy;
      }
    success=1;
    if (element>iRRAM_prec_array[1])break;
    element=element+hint;
  }
    catch ( Iteration it) {
    if (element>iRRAM_prec_array[1])break;
    element=element+hintcopy;
    }}
  if ( ! success) {
    iRRAM_DEBUG1(1,"computation of limit_hint1 failed\n");
    iRRAM_REITERATE(0);
  }
  lim.seterror(lim_error);
  iRRAM_DEBUG2(2,"end of limit_hint1 with error %d*2^(%d)\n",
               lim_error.mantissa,lim_error.exponent);
  return lim;
}


REAL limit_hint    (REAL (*f)(int, const REAL&, const REAL&),
                    int hint,
                    const REAL& x, const REAL&y)
{
  limit_computation env;

  REAL lim,limnew;
  sizetype limnew_error;
  sizetype lim_error;
  int hintcopy=hint;
  int success=0;

  int element=max(x.geterror().exponent,env.saved_prec());

  limit_debug("starting limit_hint1");

  while (1) {
    try {
    iRRAM_DEBUG2(2,"trying to compute limit_hint1 with precicion 2^(%d)...\n",element);
    limnew=f(element,x,y);
    limnew_error = sizetype_add_power2(limnew.geterror(), element);
    if ( (! success) || sizetype_less(limnew_error,lim_error) ) {
      lim=limnew;
      lim_error=limnew_error;
      iRRAM_DEBUG2(2,"getting result with error %d*2^(%d)\n",
               lim_error.mantissa, lim_error.exponent);
      } else {
      iRRAM_DEBUG1(2,"computation successful, but no improvement\n");
      hintcopy=2*hintcopy;
      }
    success=1;
    if (element>iRRAM_prec_array[1])break;
    element=element+hint;
  }
    catch ( Iteration it) {
    if (element>iRRAM_prec_array[1])break;
    element=element+hintcopy;
    }}
  if ( ! success) {
    iRRAM_DEBUG1(1,"computation of limit_hint1 failed\n");
    iRRAM_REITERATE(0);
  }
  lim.seterror(lim_error);
  iRRAM_DEBUG2(2,"end of limit_hint1 with error %d*2^(%d)\n",
                 lim_error.mantissa,lim_error.exponent);
  return lim;
}


REALMATRIX limit_lip (REALMATRIX (*f)(int,const REALMATRIX&),
            int lip,
	    bool (*on_domain)(const REALMATRIX&),
            const REALMATRIX& x)
{
  if ( on_domain(x) != true ) iRRAM_REITERATE(0);

  limit_computation env;


  REALMATRIX x_new,lim,limnew;
  sizetype lim_error;

  sizetype_exact(lim_error);
  x_new=x; x_new.seterror(lim_error);

  limit_debug("starting limit_matrix_lip1");

  while (1) {
   try {
     iRRAM_DEBUG2(2,"trying to compute limit_matrix_lip1 with precision %d...\n",actual_stack().actual_prec);
     limnew=f(env.saved_prec(),x_new);
     lim=limnew;
     lim.geterror(lim_error);
     if (lim_error.exponent > env.saved_prec()) {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip too imprecise, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
                     lim_error.mantissa,lim_error.exponent);
    break;
  }}
    catch ( Iteration it)  {
      env.inc_step(2);
      iRRAM_DEBUG2(2,"limit_lip1 failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    }}
  sizetype_exact(lim_error);
  lim.adderror(lim_error);
  lim.adderror(x.geterror() << lip);
  iRRAM_DEBUG2(2,"end of limit_matrix_lip1 with error %d*2^(%d)\n",
                 lim.geterror().mantissa,lim.geterror().exponent);

  return lim;
}


// REAL iteration (void f(REAL&,REAL&,const int& param),
//             const REAL& l,const REAL& r,const int& param)
// {
// 
//   REAL lc=l;
//   REAL rc=r;
//   ITERATION_STACK SAVED_STACK;
//   ACTUAL_STACK.inlimit+=1;
// 
//   sizetype no_error,diff_size,error;
//   sizetype_exact(no_error);
//   lc.seterror(no_error);
//   rc.seterror(no_error);
//   REAL diff;
//   REAL lcc,rcc;
// 
//   ACTUAL_STACK.actual_prec=-50;//int((upperbound(rc-lc)-50)*1);
//   ACTUAL_STACK.prec_inc=-50;
// 
// 
//   iRRAM_DEBUG1(2,"starting iteration...\n");
//   while (1) {
//      try{
//       iRRAM_DEBUG2(2,"trying to compute iteration with precision %d...\n",ACTUAL_STACK.actual_prec);
//       REAL lcc=lc;
//       REAL rcc=rc;
//       f(lcc,rcc,param);
//       lcc.geterror(error);
//       lcc.seterror(no_error);
//       lc=lcc-scale(REAL(int(error.mantissa)),error.exponent);
//       rcc.geterror(error);
//       rcc.seterror(no_error);
//       rc=rcc-scale(REAL(int(error.mantissa)),error.exponent);
// stiff_begin();stiff_begin();
//       diff=rc-lc;
// stiff_end();stiff_end();
// 
//       if (diff.vsize.exponent > SAVED_STACK.actual_prec ) {
//         iRRAM_DEBUG0(2,{ fprintf(stderr,"iteration with error %d*2^(%d)\n",
//               diff.vsize.mantissa,diff.vsize.exponent);});
//       ACTUAL_STACK.prec_inc=int(ACTUAL_STACK.prec_inc * ACTUAL_STACK.prec_factor)+iRRAM_prec_inc1;
//       ACTUAL_STACK.actual_prec=iRRAM_starting_prec+ACTUAL_STACK.prec_inc;
//       iRRAM_DEBUG2(2,"iteration result too imprecise, trying a new iteration with %d...\n",ACTUAL_STACK.actual_prec);
//     } else {
//       iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
//              diff.vsize.mantissa, diff.vsize.exponent);
//       break;
//     }}
//     catch ( Iteration it){
//       ACTUAL_STACK.prec_inc=int(ACTUAL_STACK.prec_inc * ACTUAL_STACK.prec_factor)+iRRAM_prec_inc1;
//       ACTUAL_STACK.actual_prec=iRRAM_starting_prec+ACTUAL_STACK.prec_inc;
//       iRRAM_DEBUG2(2,"iteration failed, increasing precision locally to %d...\n",ACTUAL_STACK.actual_prec);
//     } }
//       sizetype_half(diff_size,diff.vsize);
//       lc=scale(lc+rc,-1);
//       lc.adderror(diff_size);
//   iRRAM_DEBUG0(2,{ fprintf(stderr,"end of iteration with error %d*2^(%d)\n",
//               diff_size.mantissa,diff_size.exponent);});
//   return lc;
// }


REAL iteration (void (*f)(REAL&,REAL&,const int& param),
            const REAL& l,const REAL& r,const int& param)
{
  limit_computation env; //int((upperbound(rc-lc)-50)*1);

  REAL lc=l;
  REAL rc=r;
  sizetype no_error,diff_size_h,diff_size,error,diff_old_size;
  sizetype_exact(no_error);
  lc.seterror(no_error);
  rc.seterror(no_error);
  REAL diff=rc-lc;
  diff.getsize(diff_size);
  REAL lcc,rcc;


  iRRAM_DEBUG1(2,"starting iteration...\n");
  while (1) {
     try{
      iRRAM_DEBUG2(2,"trying to compute iteration with precision %d...\n",actual_stack().actual_prec);
      REAL lcc=lc;
      REAL rcc=rc;
      diff_old_size=diff_size;
      f(lcc,rcc,param);
      lcc.geterror(error);
      lcc.seterror(no_error);
      lc=lcc-scale(REAL(int(error.mantissa)),error.exponent);
      rcc.geterror(error);
      rcc.seterror(no_error);
      rc=rcc-scale(REAL(int(error.mantissa)),error.exponent);
      {
        stiff code(+2);
        diff=rc-lc;
      }
      diff.getsize(diff_size);
      if (diff_size.exponent > env.saved_prec()) {
        iRRAM_DEBUG2(2,"iteration with error %d*2^(%d)\n",
                       diff_size.mantissa,diff_size.exponent);
      if (diff_size.exponent >= diff_old_size.exponent)
        env.inc_step(2);
      iRRAM_DEBUG2(2,"iteration result too imprecise, trying a new iteration with %d...\n",actual_stack().actual_prec);
    } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             diff_size.mantissa, diff_size.exponent);
      break;
    }}
    catch ( Iteration it){
      env.inc_step(2);
      iRRAM_DEBUG2(2,"iteration failed, increasing precision locally to %d...\n",actual_stack().actual_prec);
    } }
      sizetype_half(diff_size_h,diff_size);
      lc=scale(lc+rc,-1);
      lc.adderror(diff_size_h);
      lc.geterror(error);
  iRRAM_DEBUG2(2,"end of iteration with error %d*2^(%d)\n",
                 error.mantissa,error.exponent);
  return lc;
}

//********************************************************************************
// general limit operator for FUNCTION objects on REAL numbers
//
// REAL limit ( FUNCTION<REAL,int> f )
// if FUNCTION f defines a normed Cauchy sequence, i.e. |f(i)-x|<= 2^{i}
// then limit(f) returns x
//********************************************************************************

REAL limit (const FUNCTION<REAL,int> & f )
{
  limit_computation env;

  REAL lim,limnew;
  sizetype limnew_error;
  sizetype lim_error;

  int element=env.saved_prec();
  int element_step=env.saved_step();
  int firsttime=2;

  limit_debug("starting limit_FUNCTION");

  while (1) {
    try {
    iRRAM_DEBUG2(2,"trying to compute limit_FUNCTION with precicion 2^(%d)...\n",element);
    limnew=f(element);
    limnew_error = sizetype_add_power2(limnew.geterror(), element);
    if (firsttime ==2 ) if ( limnew_error.exponent > env.saved_prec(-1)) {
      iRRAM_DEBUG2(2,"computation not precise enough (%d*2^%d), trying normal p-sequence\n",
                     limnew_error.mantissa, limnew_error.exponent);
       element_step=1;
       element=4+iRRAM_prec_array[element_step];
       firsttime=1;
    }
    if ( firsttime != 0 || sizetype_less(limnew_error,lim_error) ) {
      lim=limnew;
      lim_error=limnew_error;
      iRRAM_DEBUG2(2,"getting result with error %d*2^(%d)\n",
               lim_error.mantissa, lim_error.exponent);
      } else {
      iRRAM_DEBUG1(2,"computation successful, but no improvement\n");
      }
    firsttime=0;
    if (element<=env.saved_prec())break;
    element_step+=4;
    element=iRRAM_prec_array[element_step];
    }
    catch ( Iteration it) {
      if ( firsttime==0) {
      iRRAM_DEBUG1(2,"computation failed, using best success\n");
      break;
      } else
      if ( firsttime==2) {
      iRRAM_DEBUG1(2,"computation failed, trying normal p-sequence\n");
      element_step=1;
      element=4+iRRAM_prec_array[element_step];
      firsttime=1;
      } else {
      iRRAM_DEBUG1(1,"computation of limit_FUNCTION failed totally\n");
      iRRAM_REITERATE(0);
      }}
  }
  lim.seterror(lim_error);
  iRRAM_DEBUG2(2,"end of limit_FUNCTION with error %u*2^(%u)\n",
                 lim_error.mantissa, lim_error.exponent);
  return lim;
}

//! @}

// Instantiation of templates for multi-valued limits:
template COMPLEX limit_mv<COMPLEX,COMPLEX>
    (COMPLEX (*)(int, int *, COMPLEX const &), COMPLEX const &);

template REALMATRIX limit_mv<REALMATRIX,REALMATRIX>
    (REALMATRIX (*)(int, int *, REALMATRIX const &), REALMATRIX const &);

// Instantiation of templates for single-valued limits:

template REAL limit(REAL (*)(int, REAL const &), REAL const &);

template REAL limit(REAL (*)(int, const REAL &, const REAL &), const REAL &, const REAL &);

template COMPLEX limit(COMPLEX (*)(int, const COMPLEX &), COMPLEX const &);

template REALMATRIX limit(REALMATRIX (*)(int, const REALMATRIX &), const REALMATRIX &);

template REAL       limit(REAL (*f)(int));

template REAL       limit(REAL (*)(int, const REAL &, int), const REAL &, const int &);

} // namespace iRRAM
